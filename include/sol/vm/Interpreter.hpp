#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Environment.hpp"
#include "Value.hpp"
#include "sol/parser/Node.hpp"

namespace sol::runtime {

class Interpreter;

struct Callable {
  virtual ~Callable() = default;
  virtual Value call(Interpreter &, const std::vector<Value> &) = 0;
};

struct NativeCallable : Callable {
  std::function<Value(Interpreter &, const std::vector<Value> &)> fn;

  explicit NativeCallable(
      std::function<Value(Interpreter &, const std::vector<Value> &)> f)
      : fn(std::move(f)) {}

  Value call(Interpreter &i, const std::vector<Value> &args) override;
};

class Interpreter {
public:
  Interpreter();

  void execute(std::unique_ptr<parse::ast::Block> root);

  Value eval(parse::ast::Node &node);
  void exec(parse::ast::Node &node);

private:
  Value eval_ident(parse::ast::Ident &n);
  Value eval_string(parse::ast::String &n);
  Value eval_digit(parse::ast::Digit &n);
  Value eval_hex_digit(parse::ast::HexDigit &n);
  Value eval_binop(parse::ast::Binop &n);
  Value eval_unop(parse::ast::Unop &n);
  Value eval_func_call(parse::ast::FuncCall &n);

  void exec_block(parse::ast::Block &n);
  void exec_assign(parse::ast::Assign &n);
  void exec_decl(parse::ast::Decl &n);
  void exec_if(parse::ast::If &n);
  void exec_while(parse::ast::While &n);
  void exec_for_num(parse::ast::ForNum &n);
  void exec_return(parse::ast::Return &n);

private:
  std::shared_ptr<Environment> env;
};

} // namespace sol::runtime
