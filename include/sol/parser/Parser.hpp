#pragma once

#include <memory>
#include <string>
#include <vector>

#include "sol/parser/Node.hpp"
#include "sol/parser/lexer/Lexer.hpp"
#include "sol/parser/lexer/Token.hpp"

namespace sol::syntax {

class Parser {
public:
  Parser() = default;

  explicit Parser(lex::TokenList tokens) : _tokens(std::move(tokens)) {}

  Parser(const Parser &) = delete;
  Parser &operator=(const Parser &) = delete;

  Parser(Parser &&) = default;
  Parser &operator=(Parser &&) = default;

  ~Parser() = default;

  void set_tokens(lex::TokenList tokens) {
    _tokens = std::move(tokens);
    _cur = 0;
  }

  std::unique_ptr<ast::Block> parse();

  std::unique_ptr<ast::Block> parse(const std::string &input) {
    lex::Lexer lexer;
    _tokens = lexer.lex(input);
    _cur = 0;
    return parse();
  }

private:
  std::unique_ptr<ast::Block> parse_block();

  ast::StmtPtr parse_stmt();
  ast::ExprPtr parse_expr();
  ast::ExprPtr parse_expr_prec(int min_prec);
  ast::ExprPtr parse_simple_expr();
  ast::ExprPtr parse_prefix_expr();
  ast::ExprPtr parse_primary_expr();

  std::unique_ptr<ast::FuncDef> parse_funcbody();
  std::unique_ptr<ast::Table> parse_table();
  std::vector<ast::ExprPtr> parse_expr_list();
  std::vector<std::string> parse_name_list();
  std::vector<ast::ExprPtr> parse_args();
  std::optional<ast::Attrib> parse_attrib();
  ast::DeclName parse_decl_name();

  bool eof() const;

  bool match(std::string_view text) const;
  bool match(lex::TokenType type) const;
  bool consume(lex::TokenType t);
  bool consume(std::string_view text);
  void expect(lex::TokenType t);
  void expect(std::string_view text);
  std::string expect_name();

  const lex::Token &current() const;
  const lex::Token &peek(int n) const;
  const lex::Token &previous() const;
  const lex::Token &advance();

  static bool is_binop(std::string_view op);
  static bool is_unop(std::string_view op);
  static int precedence(std::string_view op);
  static bool right_assoc(std::string_view op);

private:
  lex::TokenList _tokens;
  std::size_t _cur = 0;
};

} // namespace sol::syntax
