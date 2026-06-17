#include "sol/parser/Node.hpp"
#include "sol/parser/Visitor.hpp"

namespace sol::parse::ast {

#define SOL_IMPL_ACCEPT(T)                                                     \
  void T::accept(Visitor &v) { v.visit(*this); }

SOL_IMPL_ACCEPT(Ident)
SOL_IMPL_ACCEPT(String)
SOL_IMPL_ACCEPT(Digit)
SOL_IMPL_ACCEPT(HexDigit)

SOL_IMPL_ACCEPT(Nil)
SOL_IMPL_ACCEPT(True)
SOL_IMPL_ACCEPT(False)

SOL_IMPL_ACCEPT(VarArg)

SOL_IMPL_ACCEPT(Binop)
SOL_IMPL_ACCEPT(Unop)

SOL_IMPL_ACCEPT(Index)
SOL_IMPL_ACCEPT(Field)

SOL_IMPL_ACCEPT(FuncCall)
SOL_IMPL_ACCEPT(MethodCall)

SOL_IMPL_ACCEPT(Table)
SOL_IMPL_ACCEPT(TableField)

SOL_IMPL_ACCEPT(Attrib)

SOL_IMPL_ACCEPT(FuncDef)

SOL_IMPL_ACCEPT(Block)

SOL_IMPL_ACCEPT(Break)
SOL_IMPL_ACCEPT(Goto)
SOL_IMPL_ACCEPT(Label)

SOL_IMPL_ACCEPT(Do)
SOL_IMPL_ACCEPT(While)
SOL_IMPL_ACCEPT(Repeat)
SOL_IMPL_ACCEPT(If)

SOL_IMPL_ACCEPT(ForNum)
SOL_IMPL_ACCEPT(ForIn)

SOL_IMPL_ACCEPT(Assign)
SOL_IMPL_ACCEPT(Decl)
SOL_IMPL_ACCEPT(Return)

SOL_IMPL_ACCEPT(Func)

SOL_IMPL_ACCEPT(ExprStmt)

#undef SOL_IMPL_ACCEPT

} // namespace sol::parse::ast
