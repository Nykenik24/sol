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

  void execute(std::unique_ptr<syntax::ast::Block> root);

  Value eval(syntax::ast::Node &node);
  void exec(syntax::ast::Node &node);

private:
  Value eval_ident(syntax::ast::Ident &n);
  Value eval_string(syntax::ast::String &n);
  Value eval_digit(syntax::ast::Digit &n);
  Value eval_hex_digit(syntax::ast::HexDigit &n);
  Value eval_binop(syntax::ast::Binop &n);
  Value eval_unop(syntax::ast::Unop &n);
  Value eval_func_call(syntax::ast::FuncCall &n);

  void exec_block(syntax::ast::Block &n);
  void exec_assign(syntax::ast::Assign &n);
  void exec_decl(syntax::ast::Decl &n);
  void exec_if(syntax::ast::If &n);
  void exec_while(syntax::ast::While &n);
  void exec_for_num(syntax::ast::ForNum &n);
  void exec_return(syntax::ast::Return &n);

private:
  std::shared_ptr<Environment> env;
};

} // namespace sol::runtime
