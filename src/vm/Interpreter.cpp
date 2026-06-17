#include "sol/vm/Interpreter.hpp"
#include "sol/error.hpp"

#include <cstdint>
#include <iostream>

namespace sol::runtime {

Value NativeCallable::call(Interpreter &i, const std::vector<Value> &args) {
  return fn(i, args);
}

static bool truthy(const Value &v) {
  if (v.is<std::monostate>())
    return false;
  if (v.is<bool>())
    return v.as<bool>();
  return true;
}

static bool equal(const Value &a, const Value &b) {
  if (a.raw().index() != b.raw().index())
    return false;

  if (a.is<std::monostate>())
    return true;
  if (a.is<bool>())
    return a.as<bool>() == b.as<bool>();
  if (a.is<double>())
    return a.as<double>() == b.as<double>();
  if (a.is<std::string>())
    return a.as<std::string>() == b.as<std::string>();

  if (a.is<std::shared_ptr<Callable>>())
    return a.as<std::shared_ptr<Callable>>() ==
           b.as<std::shared_ptr<Callable>>();

  return false;
}

Interpreter::Interpreter() : env(std::make_shared<Environment>()) {

  env->set("print",
           Value(std::make_shared<NativeCallable>(
               [](Interpreter &, const std::vector<Value> &args) -> Value {
                 for (size_t i = 0; i < args.size(); ++i) {
                   if (args[i].is<double>())
                     std::cout << args[i].as<double>();
                   else if (args[i].is<std::string>())
                     std::cout << args[i].as<std::string>();
                   else if (args[i].is<bool>())
                     std::cout << (args[i].as<bool>() ? "true" : "false");
                   else
                     std::cout << "nil";

                   if (i + 1 < args.size())
                     std::cout << '\t';
                 }
                 std::cout << '\n';
                 return Value{};
               })));
}

void Interpreter::execute(std::unique_ptr<parse::ast::Block> root) {
  exec(*root);
}

Value Interpreter::eval(parse::ast::Node &node) {
  switch (node.type()) {
  case parse::ast::NodeType::Ident:
    return eval_ident(static_cast<parse::ast::Ident &>(node));

  case parse::ast::NodeType::String:
    return eval_string(static_cast<parse::ast::String &>(node));

  case parse::ast::NodeType::Digit:
    return eval_digit(static_cast<parse::ast::Digit &>(node));

  case parse::ast::NodeType::HexDigit:
    return eval_hex_digit(static_cast<parse::ast::HexDigit &>(node));

  case parse::ast::NodeType::Nil:
    return Value{};

  case parse::ast::NodeType::True:
    return Value(true);

  case parse::ast::NodeType::False:
    return Value(false);

  case parse::ast::NodeType::Binop:
    return eval_binop(static_cast<parse::ast::Binop &>(node));

  case parse::ast::NodeType::Unop:
    return eval_unop(static_cast<parse::ast::Unop &>(node));

  case parse::ast::NodeType::FuncCall:
    return eval_func_call(static_cast<parse::ast::FuncCall &>(node));

  default:
    error::fatal("unsupported expression node {}\n",
                 static_cast<int>(node.type()));
  }
}

void Interpreter::exec(parse::ast::Node &node) {
  switch (node.type()) {
  case parse::ast::NodeType::Block:
    exec_block(static_cast<parse::ast::Block &>(node));
    return;

  case parse::ast::NodeType::Assign:
    exec_assign(static_cast<parse::ast::Assign &>(node));
    return;

  case parse::ast::NodeType::Decl:
    exec_decl(static_cast<parse::ast::Decl &>(node));
    return;

  case parse::ast::NodeType::If:
    exec_if(static_cast<parse::ast::If &>(node));
    return;

  case parse::ast::NodeType::While:
    exec_while(static_cast<parse::ast::While &>(node));
    return;

  case parse::ast::NodeType::ForNum:
    exec_for_num(static_cast<parse::ast::ForNum &>(node));
    return;

  case parse::ast::NodeType::ExprStmt:
    eval(*static_cast<parse::ast::ExprStmt &>(node).expr);
    return;

  case parse::ast::NodeType::Return:
    exec_return(static_cast<parse::ast::Return &>(node));
    return;

  default:
    error::fatal("unsupported statement node {}\n",
                 static_cast<int>(node.type()));
  }
}

Value Interpreter::eval_ident(parse::ast::Ident &n) { return env->get(n.name); }

Value Interpreter::eval_string(parse::ast::String &n) { return Value(n.str); }

Value Interpreter::eval_digit(parse::ast::Digit &n) { return Value(n.value); }

Value Interpreter::eval_hex_digit(parse::ast::HexDigit &n) {
  return Value(n.value);
}

Value Interpreter::eval_binop(parse::ast::Binop &n) {
  Value l = eval(*n.lhs);
  Value r = eval(*n.rhs);

  if (l.is<double>() && r.is<double>()) {
    double a = l.as<double>();
    double b = r.as<double>();

    switch (n.op.type()) {
    case lex::TokenType::Add:
      return Value(a + b);
    case lex::TokenType::Sub:
      return Value(a - b);
    case lex::TokenType::Mul:
      return Value(a * b);
    case lex::TokenType::Div:
      return Value(a / b);
    case lex::TokenType::Less:
      return Value(a < b);
    case lex::TokenType::More:
      return Value(a > b);
    case lex::TokenType::LessEqual:
      return Value(a <= b);
    case lex::TokenType::MoreEqual:
      return Value(a >= b);
    default:
      break;
    }
  }

  if (l.is<std::string>() && r.is<std::string>()) {
    if (n.op.type() == lex::TokenType::Equal)
      return Value(l.as<std::string>() == r.as<std::string>());
    if (n.op.type() == lex::TokenType::NotEqual)
      return Value(l.as<std::string>() != r.as<std::string>());
  }

  if (n.op.type() == lex::TokenType::And)
    return Value(truthy(l) && truthy(r));

  if (n.op.type() == lex::TokenType::Or)
    return Value(truthy(l) || truthy(r));

  if (n.op.type() == lex::TokenType::Equal)
    return Value(equal(l, r));

  if (n.op.type() == lex::TokenType::NotEqual)
    return Value(!equal(l, r));

  error::fatal("unsupported binary op '{}' for {} and {}\n", n.op.txt(),
               l.type_as_str(), r.type_as_str());
}

Value Interpreter::eval_unop(parse::ast::Unop &n) {
  Value v = eval(*n.operand);

  if (n.op.type() == lex::TokenType::Sub)
    return Value(-v.as<double>());

  if (n.op.type() == lex::TokenType::BitNot)
    return Value((double)~(int64_t)v.as<double>());

  error::fatal("unsupported unary op '{}' for {}\n", n.op.txt(),
               v.type_as_str());
}

Value Interpreter::eval_func_call(parse::ast::FuncCall &n) {
  Value target = eval(*n.target);

  std::vector<Value> args;
  args.reserve(n.args.size());

  for (auto &a : n.args)
    args.push_back(eval(*a));

  auto fn = std::get<std::shared_ptr<Callable>>(target.raw());
  return fn->call(*this, args);
}

void Interpreter::exec_block(parse::ast::Block &n) {
  auto prev = env;
  env = std::make_shared<Environment>(prev);

  for (auto &s : n.stmts)
    exec(*s);

  env = prev;
}

void Interpreter::exec_assign(parse::ast::Assign &n) {
  for (size_t i = 0; i < n.targets.size(); ++i) {
    Value v = eval(*n.values[i]);

    auto &t = n.targets[i];
    if (t->type() == parse::ast::NodeType::Ident) {
      auto &id = static_cast<parse::ast::Ident &>(*t);
      env->set(id.name, v);
    }
  }
}

void Interpreter::exec_decl(parse::ast::Decl &n) {
  for (size_t i = 0; i < n.names.size(); ++i) {
    Value v{};
    if (i < n.values.size())
      v = eval(*n.values[i]);

    env->set(n.names[i].name, v);
  }
}

void Interpreter::exec_if(parse::ast::If &n) {
  for (auto &b : n.branches) {
    if (truthy(eval(*b.cond))) {
      exec(*b.body);
      return;
    }
  }

  if (n.elseBody)
    exec(*n.elseBody);
}

void Interpreter::exec_while(parse::ast::While &n) {
  while (truthy(eval(*n.cond)))
    exec(*n.body);
}

void Interpreter::exec_for_num(parse::ast::ForNum &n) {
  double start = eval(*n.start).as<double>();
  double limit = eval(*n.limit).as<double>();
  double step = eval(*n.step).as<double>();

  for (double i = start; i <= limit; i += step) {
    env->set(n.name, Value(i));
    exec(*n.body);
  }
}

void Interpreter::exec_return(parse::ast::Return &n) {
  throw eval(*n.values[0]);
}

} // namespace sol::runtime
