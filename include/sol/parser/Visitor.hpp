#pragma once

#include "sol/parser/Node.hpp"

namespace sol::syntax::ast {
class Visitor {
public:
  virtual ~Visitor() = default;

  virtual void visit(Ident &) = 0;
  virtual void visit(String &) = 0;
  virtual void visit(Digit &) = 0;
  virtual void visit(HexDigit &) = 0;

  virtual void visit(Nil &) = 0;
  virtual void visit(True &) = 0;
  virtual void visit(False &) = 0;

  virtual void visit(VarArg &) = 0;

  virtual void visit(Binop &) = 0;
  virtual void visit(Unop &) = 0;

  virtual void visit(Index &) = 0;
  virtual void visit(Field &) = 0;

  virtual void visit(FuncCall &) = 0;
  virtual void visit(MethodCall &) = 0;

  virtual void visit(Table &) = 0;
  virtual void visit(TableField &) = 0;

  virtual void visit(Attrib &) = 0;

  virtual void visit(FuncDef &) = 0;

  virtual void visit(Block &) = 0;

  virtual void visit(Break &) = 0;
  virtual void visit(Goto &) = 0;
  virtual void visit(Label &) = 0;

  virtual void visit(Do &) = 0;
  virtual void visit(While &) = 0;
  virtual void visit(Repeat &) = 0;
  virtual void visit(If &) = 0;

  virtual void visit(ForNum &) = 0;
  virtual void visit(ForIn &) = 0;

  virtual void visit(Assign &) = 0;
  virtual void visit(Decl &) = 0;
  virtual void visit(Return &) = 0;

  virtual void visit(Func &) = 0;

  virtual void visit(ExprStmt &) = 0;
};
} // namespace sol::syntax::ast
