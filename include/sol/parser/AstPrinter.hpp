#pragma once

#include <iostream>

#include "sol/parser/Visitor.hpp"

namespace sol::syntax::ast {

class AstPrinter final : public Visitor {
public:
  explicit AstPrinter(std::ostream &out = std::cout) : out(out) {}

private:
  std::ostream &out;
  int depth = 0;

  void indent() {
    for (int i = 0; i < depth; ++i)
      out << "  ";
  }

  void print(Node &node) { node.accept(*this); }

public:
  void visit(Ident &n) override;
  void visit(String &n) override;
  void visit(Digit &n) override;
  void visit(HexDigit &n) override;

  void visit(Nil &n) override;
  void visit(True &n) override;
  void visit(False &n) override;

  void visit(VarArg &n) override;

  void visit(Binop &n) override;
  void visit(Unop &n) override;

  void visit(Index &n) override;
  void visit(Field &n) override;

  void visit(FuncCall &n) override;
  void visit(MethodCall &n) override;

  void visit(Table &n) override;
  void visit(TableField &n) override;

  void visit(Attrib &n) override;

  void visit(FuncDef &n) override;

  void visit(Block &n) override;

  void visit(Break &n) override;
  void visit(Goto &n) override;
  void visit(Label &n) override;

  void visit(Do &n) override;
  void visit(While &n) override;
  void visit(Repeat &n) override;
  void visit(If &n) override;

  void visit(ForNum &n) override;
  void visit(ForIn &n) override;

  void visit(Assign &n) override;
  void visit(Decl &n) override;
  void visit(Return &n) override;

  void visit(Func &n) override;

  void visit(ExprStmt &n) override;
};

} // namespace sol::syntax::ast
