#include "sol/parser/Parser.hpp"
#include "sol/error.hpp"
#include "sol/lexer/Token.hpp"
#include "sol/parser/Node.hpp"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sol::parse {
bool Parser::eof() const {
  return _cur >= _tokens.size() || _tokens[_cur].type() == lex::TokenType::Eof;
}

const lex::Token &Parser::current() const { return _tokens[_cur]; }
const lex::Token &Parser::peek(int n) const { return _tokens[_cur + n]; }
const lex::Token &Parser::previous() const { return _tokens[_cur - 1]; }
const lex::Token &Parser::advance() { return _tokens[_cur++]; }

bool Parser::match(lex::TokenType t) const {
  return !eof() && current().type() == t;
}

bool Parser::match(std::string_view txt) const {
  return !eof() && current().txt().compare(txt) == 0;
}

bool Parser::consume(lex::TokenType t) {
  if (!match(t))
    return false;

  advance();
  return true;
}

bool Parser::consume(std::string_view t) {
  if (!match(t))
    return false;

  advance();
  return true;
}

void Parser::expect(lex::TokenType t) {
  if (!match(t))
    throw std::runtime_error("expected token");
  advance();
}

void Parser::expect(std::string_view t) {
  if (!match(t))
    throw std::runtime_error("expected token");
  advance();
}

std::string Parser::expect_name() {
  if (!match(lex::TokenType::Ident))
    throw std::runtime_error("expected identifier");

  auto name = current().txt();

  advance();

  return name;
}

bool Parser::is_unop(std::string_view op) {
  return op.compare("-") == 0 || op.compare("not") == 0 ||
         op.compare("#") == 0 || op.compare("~") == 0;
}

bool Parser::is_binop(std::string_view txt) {
  static std::vector<std::string_view> ops = {
      "+",  "-",  "*", "/",  "//", "^",  "%",  "&",  "~",   "|", ">>",
      "<<", "..", "<", "<=", ">",  ">=", "==", "~=", "and", "or"};
  for (const auto op : ops)
    if (txt.compare(op) == 0)
      return 1;
  return 0;
}

std::vector<ast::ExprPtr> Parser::parse_args() {
  std::vector<ast::ExprPtr> args;

  if (match(lex::TokenType::LeftParen)) {
    advance();
    if (!match(lex::TokenType::RightParen)) {
      args.push_back(parse_expr());
      while (match(lex::TokenType::Comma)) {
        advance();
        args.push_back(parse_expr());
      }
    }
    expect(lex::TokenType::RightParen);
  } else if (match(lex::TokenType::LeftBrace)) {
    args.push_back(parse_table());
  } else if (match(lex::TokenType::String)) {
    args.push_back(std::make_unique<ast::String>(current().txt()));
  } else {
    error::fatal("expected function arguments '{}'\n", current().txt());
  }

  return args;
}

std::optional<ast::Attrib> Parser::parse_attrib() {
  if (!match(lex::TokenType::Less))
    return std::nullopt;

  advance();
  auto name = expect_name();
  expect(lex::TokenType::More);
  return ast::Attrib(std::move(name));
}

std::vector<ast::ExprPtr> Parser::parse_expr_list() {
  std::vector<ast::ExprPtr> exprs;
  exprs.push_back(parse_expr());
  while (consume(lex::TokenType::Comma)) {
    exprs.push_back(parse_expr());
  }
  return exprs;
}

std::vector<std::string> Parser::parse_name_list() {
  std::vector<std::string> names;
  names.push_back(expect_name());
  while (consume(lex::TokenType::Comma)) {
    names.push_back(expect_name());
  }
  return names;
}

std::unique_ptr<ast::Table> Parser::parse_table() {
  expect(lex::TokenType::LeftBrace);
  std::vector<std::unique_ptr<ast::TableField>> fields;

  while (!match(lex::TokenType::RightBrace)) {
    if (consume(lex::TokenType::LeftBracket)) {
      auto key = parse_expr();
      expect(lex::TokenType::RightBracket);
      expect(lex::TokenType::Assign);
      auto val = parse_expr();
      fields.push_back(
          std::make_unique<ast::TableField>(std::move(key), std::move(val)));
    } else if (match(lex::TokenType::Ident) && !eof() &&
               peek(1).txt().compare("=") == 0) {
      auto name = current().txt();
      advance();
      advance(); // =
      auto val = parse_expr();
      fields.push_back(std::make_unique<ast::TableField>(
          std::move(std::make_unique<ast::Ident>(name)), std::move(val)));
    } else {
      auto val = parse_expr();
      fields.push_back(
          std::make_unique<ast::TableField>(nullptr, std::move(val)));
    }

    if (match(lex::TokenType::Comma) || match(lex::TokenType::Semicolon))
      advance();
    else
      break;
  }

  expect(lex::TokenType::RightBrace);

  auto t = std::make_unique<ast::Table>(std::move(fields));
  return t;
}

ast::ExprPtr Parser::parse_primary_expr() {
  if (match(lex::TokenType::Ident)) {
    auto name = current().txt();
    advance();

    return std::make_unique<ast::Ident>(std::move(name));
  }

  if (consume(lex::TokenType::LeftParen)) {
    auto expr = parse_expr();
    expect(lex::TokenType::RightParen);

    return expr;
  }

  error::fatal("expected expression");
}

ast::ExprPtr Parser::parse_prefix_expr() {
  auto expr = parse_primary_expr();

  while (!eof()) {

    if (consume(lex::TokenType::Period)) {
      auto field = expect_name();

      expr = std::make_unique<ast::Field>(std::move(expr), std::move(field));

      continue;
    }

    if (consume(lex::TokenType::LeftBracket)) {
      auto key = parse_expr();

      expect(lex::TokenType::RightBracket);

      expr = std::make_unique<ast::Index>(std::move(expr), std::move(key));

      continue;
    }

    if (consume(lex::TokenType::Colon)) {
      auto method = expect_name();

      auto call =
          std::make_unique<ast::MethodCall>(std::move(expr), std::move(method));

      call->args = parse_args();

      expr = std::move(call);

      continue;
    }

    if (match(lex::TokenType::LeftParen) || match(lex::TokenType::LeftBrace) ||
        match(lex::TokenType::String)) {

      auto call = std::make_unique<ast::FuncCall>(std::move(expr));

      call->args = parse_args();

      expr = std::move(call);

      continue;
    }

    break;
  }

  return expr;
}

ast::DeclName Parser::parse_decl_name() {
  auto pre = parse_attrib();
  auto name = expect_name();
  auto post = parse_attrib();

  ast::DeclName dn;
  dn.name = std::move(name);

  if (pre)
    dn.attrib = std::move(*pre);
  else if (post)
    dn.attrib = std::move(*post);

  return dn;
}

ast::ExprPtr Parser::parse_simple_expr() {
  if (consume(lex::TokenType::Nil))
    return std::make_unique<ast::Nil>();
  if (consume(lex::TokenType::True))
    return std::make_unique<ast::True>();
  if (consume(lex::TokenType::False))
    return std::make_unique<ast::False>();
  if (consume(lex::TokenType::VarArg))
    return std::make_unique<ast::VarArg>();
  if (match(lex::TokenType::Digit)) {
    auto value = std::stod(current().txt());
    advance();
    return std::make_unique<ast::Digit>(value);
  }
  if (match(lex::TokenType::HexDigit)) {
    double d;
    try {
      *reinterpret_cast<unsigned long long *>(&d) =
          std::stoull(current().txt(), nullptr, 16);
    } catch (...) {
    }

    advance();
    return std::make_unique<ast::HexDigit>(d);
  }
  if (consume(lex::TokenType::String))
    return std::make_unique<ast::String>(previous().txt());

  if (match(lex::TokenType::Function)) {
    advance();
    return parse_funcbody();
  }

  if (match(lex::TokenType::LeftBrace))
    return parse_table();

  if (is_unop(current().txt())) {
    auto op = current();
    advance();
    auto operand = parse_simple_expr();

    return std::make_unique<ast::Unop>(std::move(operand), op);
  }

  if (consume(lex::TokenType::Local)) {
    expect(lex::TokenType::Function);
    auto name = expect_name();
    return parse_funcbody();
  }

  return parse_prefix_expr();
}

int Parser::precedence(std::string_view op) {
  if (op.compare("or") == 0)
    return 1;
  if (op.compare("and") == 0)
    return 2;
  if (op.compare("<") == 0 == 0 || op.compare(">") == 0 == 0 ||
      op.compare("<=") == 0 || op.compare(">=") == 0 == 0 ||
      op.compare("==") == 0 == 0 || op.compare("~=") == 0)
    return 3;
  if (op.compare("|") == 0)
    return 4;
  if (op.compare("~") == 0)
    return 5;
  if (op.compare("&") == 0)
    return 6;
  if (op.compare("<<") == 0 == 0 || op.compare(">>") == 0)
    return 7;
  if (op.compare("..") == 0)
    return 8;
  if (op.compare("+") == 0 == 0 || op.compare("-") == 0)
    return 9;
  if (op.compare("*") == 0 == 0 || op.compare("/") == 0 == 0 ||
      op.compare("//") == 0 || op.compare("%") == 0)
    return 10;
  if (op.compare("^") == 0)
    return 12;
  return -1;
}

bool Parser::right_assoc(std::string_view op) {
  return op.compare("^") == 0 || op.compare("..") == 0;
}

ast::ExprPtr Parser::parse_expr_prec(int min_prec) {
  auto left = parse_simple_expr();

  while (!eof() && is_binop(current().txt())) {
    auto op = current();
    int prec = precedence(op.txt());
    if (prec < min_prec)
      break;

    advance();

    int next_prec = right_assoc(op.txt()) ? prec : prec + 1;
    auto right = parse_expr_prec(next_prec);

    left = std::make_unique<ast::Binop>(std::move(left), std::move(right),
                                        std::move(op));
  }

  return left;
}

ast::ExprPtr Parser::parse_expr() { return parse_expr_prec(1); }

ast::StmtPtr Parser::parse_stmt() {
  if (consume(lex::TokenType::Semicolon))
    return nullptr;

  if (consume(lex::TokenType::Break))
    return std::make_unique<ast::Break>();

  if (consume(lex::TokenType::Goto))
    return std::make_unique<ast::Goto>(expect_name());

  if (consume(lex::TokenType::Label)) {
    auto name = expect_name();

    expect(lex::TokenType::Label);

    return std::make_unique<ast::Label>(std::move(name));
  }

  if (consume(lex::TokenType::Do)) {
    auto body = parse_block();

    expect(lex::TokenType::End);

    return std::make_unique<ast::Do>(std::move(body));
  }

  if (consume(lex::TokenType::While)) {
    auto cond = parse_expr();

    expect(lex::TokenType::Do);

    auto body = parse_block();

    expect(lex::TokenType::End);

    return std::make_unique<ast::While>(std::move(cond), std::move(body));
  }

  if (consume(lex::TokenType::Repeat)) {
    auto body = parse_block();

    expect(lex::TokenType::Until);

    auto cond = parse_expr();

    return std::make_unique<ast::Repeat>(std::move(body), std::move(cond));
  }

  if (consume(lex::TokenType::If)) {
    auto node = std::make_unique<ast::If>();

    {
      ast::IfBranch branch;
      branch.cond = parse_expr();

      expect(lex::TokenType::Then);

      branch.body = parse_block();

      node->branches.push_back(std::move(branch));
    }

    while (consume(lex::TokenType::Elseif)) {
      ast::IfBranch branch;

      branch.cond = parse_expr();

      expect(lex::TokenType::Then);

      branch.body = parse_block();

      node->branches.push_back(std::move(branch));
    }

    if (consume(lex::TokenType::Else))
      node->elseBody = parse_block();

    expect(lex::TokenType::End);

    return node;
  }

  if (consume(lex::TokenType::For)) {
    auto first = expect_name();

    if (consume(lex::TokenType::Assign)) {
      auto node = std::make_unique<ast::ForNum>(std::move(first));

      node->start = parse_expr();

      expect(lex::TokenType::Comma);

      node->limit = parse_expr();

      if (consume(lex::TokenType::Comma))
        node->step = parse_expr();

      expect(lex::TokenType::Do);

      node->body = parse_block();

      expect(lex::TokenType::End);

      return node;
    }

    auto node = std::make_unique<ast::ForIn>();

    node->names.push_back(std::move(first));

    while (consume(lex::TokenType::Comma))
      node->names.push_back(expect_name());

    expect(lex::TokenType::In);

    node->iterators = parse_expr_list();

    expect(lex::TokenType::Do);

    node->body = parse_block();

    expect(lex::TokenType::End);

    return node;
  }

  if (consume(lex::TokenType::Function)) {
    auto func = std::make_unique<ast::Func>();

    func->path.push_back(expect_name());

    while (consume(lex::TokenType::Period))
      func->path.push_back(expect_name());

    if (consume(lex::TokenType::Colon))
      func->method = expect_name();

    func->body = parse_funcbody();

    return func;
  }

  if (consume(lex::TokenType::Var)) {
    auto decl = std::make_unique<ast::Decl>();

    decl->names.push_back(parse_decl_name());

    while (consume(lex::TokenType::Comma))
      decl->names.push_back(parse_decl_name());

    if (consume(lex::TokenType::Assign))
      decl->values = parse_expr_list();

    return decl;
  }

  auto first = parse_prefix_expr();

  if (match(lex::TokenType::Assign) || match(lex::TokenType::Comma)) {

    auto assign = std::make_unique<ast::Assign>();

    auto *lvalue = dynamic_cast<ast::LValue *>(first.release());

    if (!lvalue)
      error::fatal("assignment target must be lvalue");

    assign->targets.emplace_back(lvalue);

    while (consume(lex::TokenType::Comma)) {

      auto target = parse_prefix_expr();

      auto *lv = dynamic_cast<ast::LValue *>(target.release());

      if (!lv)
        error::fatal("assignment target must be lvalue");

      assign->targets.emplace_back(lv);
    }

    expect(lex::TokenType::Assign);

    assign->values = parse_expr_list();

    return assign;
  }

  if (first->is(ast::NodeType::FuncCall) ||
      first->is(ast::NodeType::MethodCall)) {

    return std::make_unique<ast::ExprStmt>(std::move(first));
  }

  error::fatal("unexpected statement");
}

std::unique_ptr<ast::FuncDef> Parser::parse_funcbody() {
  expect(lex::TokenType::LeftParen);

  std::vector<std::string> params;
  bool has_vararg = false;
  std::optional<std::string> vararg_name;

  if (!match(lex::TokenType::RightParen)) {
    if (consume(lex::TokenType::VarArg)) {
      has_vararg = true;
      if (consume(lex::TokenType::Ident))
        vararg_name = previous().txt();
    } else {
      params.push_back(expect_name());
      while (consume(lex::TokenType::Comma)) {
        if (consume(lex::TokenType::VarArg)) {
          has_vararg = true;
          if (consume(lex::TokenType::Ident))
            vararg_name = previous().txt();
          break;
        }
        params.push_back(expect_name());
      }
    }
  }

  expect(lex::TokenType::RightParen);
  auto body = parse_block();
  expect(lex::TokenType::End);

  auto fd = std::make_unique<ast::FuncDef>();
  fd->hasVararg = std::move(has_vararg);
  fd->varargName = std::move(vararg_name);
  fd->params = std::move(params);
  fd->body = std::move(body);
  return fd;
}

std::unique_ptr<ast::Block> Parser::parse_block() {
  std::vector<ast::StmtPtr> stmts;

  while (!eof() && !match(lex::TokenType::End) &&
         !match(lex::TokenType::Else) && !match(lex::TokenType::Elseif) &&
         !match(lex::TokenType::Until)) {
    stmts.push_back(parse_stmt());
  }

  return std::make_unique<ast::Block>(std::move(stmts));
}

std::unique_ptr<ast::Block> Parser::parse() {
  auto block = std::make_unique<ast::Block>();
  while (!eof()) {
    block->stmts.push_back(parse_stmt());
  }
  return block;
}
} // namespace sol::parse
