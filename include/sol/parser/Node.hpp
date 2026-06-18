#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "sol/parser/lexer/Token.hpp"

namespace sol::syntax::ast {

class Visitor;

enum class NodeType {
  Ident,
  String,
  Digit,
  HexDigit,
  Nil,
  True,
  False,
  Vararg,

  Binop,
  Unop,

  Break,
  Goto,
  Label,

  Block,

  Do,
  While,
  Repeat,
  If,

  ForNum,
  ForIn,

  Return,
  Assign,
  Decl,

  Func,
  FuncDef,

  FuncCall,
  MethodCall,

  Index,
  Field,

  Table,
  TableField,

  Attrib,

  ExprStmt,
};

class Node {
public:
  explicit Node(NodeType type) : _type(type) {}
  virtual ~Node() = default;

  NodeType type() const { return _type; }
  bool is(NodeType t) const { return _type == t; }

  virtual void accept(Visitor &v) = 0;

private:
  NodeType _type;
};

class Expr : public Node {
public:
  using Node::Node;
};

class Stmt : public Node {
public:
  using Node::Node;
};

class LValue : public Expr {
public:
  using Expr::Expr;
};

class Literal : public Expr {
public:
  using Expr::Expr;
};

using NodePtr = std::unique_ptr<Node>;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using LValuePtr = std::unique_ptr<LValue>;

#define SOL_ACCEPT() void accept(Visitor &v) override;

class Ident final : public LValue {
public:
  explicit Ident(std::string name)
      : LValue(NodeType::Ident), name(std::move(name)) {}

  std::string name;

  SOL_ACCEPT()
};

class String final : public Literal {
public:
  explicit String(std::string str)
      : Literal(NodeType::String), str(std::move(str)) {}

  std::string str;

  SOL_ACCEPT()
};

class Digit final : public Literal {
public:
  explicit Digit(double value) : Literal(NodeType::Digit), value(value) {}

  double value;

  SOL_ACCEPT()
};

class HexDigit final : public Literal {
public:
  explicit HexDigit(double value) : Literal(NodeType::HexDigit), value(value) {}

  double value;

  SOL_ACCEPT()
};

class Nil final : public Literal {
public:
  Nil() : Literal(NodeType::Nil) {}

  SOL_ACCEPT()
};

class True final : public Literal {
public:
  True() : Literal(NodeType::True) {}

  SOL_ACCEPT()
};

class False final : public Literal {
public:
  False() : Literal(NodeType::False) {}

  SOL_ACCEPT()
};

class VarArg final : public Expr {
public:
  VarArg() : Expr(NodeType::Vararg) {}

  SOL_ACCEPT()
};

class Binop final : public Expr {
public:
  Binop(ExprPtr lhs, ExprPtr rhs, lex::Token op)
      : Expr(NodeType::Binop), lhs(std::move(lhs)), rhs(std::move(rhs)),
        op(std::move(op)) {}

  ExprPtr lhs;
  ExprPtr rhs;
  lex::Token op;

  SOL_ACCEPT()
};

class Unop final : public Expr {
public:
  Unop(ExprPtr operand, lex::Token op)
      : Expr(NodeType::Unop), operand(std::move(operand)), op(std::move(op)) {}

  ExprPtr operand;
  lex::Token op;

  SOL_ACCEPT()
};

class Index final : public LValue {
public:
  Index(ExprPtr target, ExprPtr key)
      : LValue(NodeType::Index), target(std::move(target)),
        key(std::move(key)) {}

  ExprPtr target;
  ExprPtr key;

  SOL_ACCEPT()
};

class Field final : public LValue {
public:
  Field(ExprPtr target, std::string name)
      : LValue(NodeType::Field), target(std::move(target)),
        name(std::move(name)) {}

  ExprPtr target;
  std::string name;

  SOL_ACCEPT()
};

class FuncCall final : public Expr {
public:
  explicit FuncCall(ExprPtr target)
      : Expr(NodeType::FuncCall), target(std::move(target)) {}

  ExprPtr target;
  std::vector<ExprPtr> args;

  SOL_ACCEPT()
};

class MethodCall final : public Expr {
public:
  MethodCall(ExprPtr target, std::string method)
      : Expr(NodeType::MethodCall), target(std::move(target)),
        method(std::move(method)) {}

  ExprPtr target;
  std::string method;
  std::vector<ExprPtr> args;

  SOL_ACCEPT()
};

class TableField final : public Node {
public:
  TableField(ExprPtr key, ExprPtr value)
      : Node(NodeType::TableField), key(std::move(key)),
        value(std::move(value)) {}

  ExprPtr key;
  ExprPtr value;

  SOL_ACCEPT()
};

class Table final : public Expr {
public:
  explicit Table(std::vector<std::unique_ptr<TableField>> fields)
      : Expr(NodeType::Table), fields(std::move(fields)) {}

  std::vector<std::unique_ptr<TableField>> fields;

  SOL_ACCEPT()
};

class Attrib final : public Expr {
public:
  explicit Attrib(const std::string &name)
      : Expr(NodeType::Attrib), name(name) {}

  std::string name;

  SOL_ACCEPT()
};

class Block final : public Node {
public:
  Block() : Node(NodeType::Block) {}

  explicit Block(std::vector<StmtPtr> stmts)
      : Node(NodeType::Block), stmts(std::move(stmts)) {}

  std::vector<StmtPtr> stmts;

  SOL_ACCEPT()
};

class FuncDef final : public Expr {
public:
  FuncDef() : Expr(NodeType::FuncDef) {}

  std::vector<std::string> params;

  bool hasVararg = false;
  std::optional<std::string> varargName;

  std::unique_ptr<Block> body;

  SOL_ACCEPT()
};

class Break final : public Stmt {
public:
  Break() : Stmt(NodeType::Break) {}

  SOL_ACCEPT()
};

class Goto final : public Stmt {
public:
  explicit Goto(std::string label)
      : Stmt(NodeType::Goto), label(std::move(label)) {}

  std::string label;

  SOL_ACCEPT()
};

class Label final : public Stmt {
public:
  explicit Label(std::string name)
      : Stmt(NodeType::Label), name(std::move(name)) {}

  std::string name;

  SOL_ACCEPT()
};

class Do final : public Stmt {
public:
  explicit Do(std::unique_ptr<Block> body)
      : Stmt(NodeType::Do), body(std::move(body)) {}

  std::unique_ptr<Block> body;

  SOL_ACCEPT()
};

class While final : public Stmt {
public:
  While(ExprPtr cond, std::unique_ptr<Block> body)
      : Stmt(NodeType::While), cond(std::move(cond)), body(std::move(body)) {}

  ExprPtr cond;
  std::unique_ptr<Block> body;

  SOL_ACCEPT()
};

class Repeat final : public Stmt {
public:
  Repeat(std::unique_ptr<Block> body, ExprPtr cond)
      : Stmt(NodeType::Repeat), body(std::move(body)), cond(std::move(cond)) {}

  std::unique_ptr<Block> body;
  ExprPtr cond;

  SOL_ACCEPT()
};

struct IfBranch {
  ExprPtr cond;
  std::unique_ptr<Block> body;
};

class If final : public Stmt {
public:
  If() : Stmt(NodeType::If) {}

  std::vector<IfBranch> branches;
  std::unique_ptr<Block> elseBody;

  SOL_ACCEPT()
};

class ForNum final : public Stmt {
public:
  explicit ForNum(std::string name)
      : Stmt(NodeType::ForNum), name(std::move(name)) {}

  std::string name;

  ExprPtr start;
  ExprPtr limit;
  ExprPtr step;

  std::unique_ptr<Block> body;

  SOL_ACCEPT()
};

class ForIn final : public Stmt {
public:
  ForIn() : Stmt(NodeType::ForIn) {}

  std::vector<std::string> names;
  std::vector<ExprPtr> iterators;

  std::unique_ptr<Block> body;

  SOL_ACCEPT()
};

class Assign final : public Stmt {
public:
  Assign() : Stmt(NodeType::Assign) {}

  std::vector<LValuePtr> targets;
  std::vector<ExprPtr> values;

  SOL_ACCEPT()
};

struct DeclName {
  std::string name;
  std::optional<Attrib> attrib;
};

class Decl final : public Stmt {
public:
  explicit Decl() : Stmt(NodeType::Decl) {}

  std::vector<DeclName> names;
  std::vector<ExprPtr> values;

  SOL_ACCEPT()
};

class Return final : public Stmt {
public:
  Return() : Stmt(NodeType::Return) {}

  std::vector<ExprPtr> values;

  SOL_ACCEPT()
};

class Func final : public Stmt {
public:
  Func() : Stmt(NodeType::Func) {}

  std::vector<std::string> path;
  std::optional<std::string> method;
  std::unique_ptr<FuncDef> body;

  SOL_ACCEPT()
};

class ExprStmt final : public Stmt {
public:
  explicit ExprStmt(ExprPtr expr)
      : Stmt(NodeType::ExprStmt), expr(std::move(expr)) {}

  ExprPtr expr;

  SOL_ACCEPT()
};

#undef SOL_ACCEPT

} // namespace sol::syntax::ast
