#include "sol/parser/AstPrinter.hpp"

namespace sol::syntax::ast {

void AstPrinter::visit(Ident &n) {
  indent();
  out << "ident(" << n.name << ")\n";
}

void AstPrinter::visit(String &n) {
  indent();
  out << "string(" << n.str << ")\n";
}

void AstPrinter::visit(Digit &n) {
  indent();
  out << "digit(" << n.value << ")\n";
}

void AstPrinter::visit(HexDigit &n) {
  indent();
  out << "hex(" << n.value << ")\n";
}

void AstPrinter::visit(Nil &) {
  indent();
  out << "nil\n";
}

void AstPrinter::visit(True &) {
  indent();
  out << "true\n";
}

void AstPrinter::visit(False &) {
  indent();
  out << "false\n";
}

void AstPrinter::visit(VarArg &) {
  indent();
  out << "...\n";
}

void AstPrinter::visit(Binop &n) {
  indent();
  out << "binop(" << n.op.txt() << ")\n";

  depth++;
  print(*n.lhs);
  print(*n.rhs);
  depth--;
}

void AstPrinter::visit(Unop &n) {
  indent();
  out << "unop(" << n.op.txt() << ")\n";

  depth++;
  print(*n.operand);
  depth--;
}

void AstPrinter::visit(Index &n) {
  indent();
  out << "index\n";

  depth++;

  indent();
  out << "target:\n";

  depth++;
  print(*n.target);
  depth--;

  indent();
  out << "key:\n";

  depth++;
  print(*n.key);
  depth--;

  depth--;
}

void AstPrinter::visit(Field &n) {
  indent();
  out << "field(." << n.name << ")\n";

  depth++;

  indent();
  out << "target:\n";

  depth++;
  print(*n.target);
  depth--;

  depth--;
}

void AstPrinter::visit(FuncCall &n) {
  indent();
  out << "call\n";

  depth++;

  indent();
  out << "target:\n";

  depth++;
  print(*n.target);
  depth--;

  if (!n.args.empty()) {
    indent();
    out << "args:\n";

    depth++;

    for (auto &arg : n.args)
      print(*arg);

    depth--;
  }

  depth--;
}

void AstPrinter::visit(MethodCall &n) {
  indent();
  out << "method_call(:" << n.method << ")\n";

  depth++;

  indent();
  out << "target:\n";

  depth++;
  print(*n.target);
  depth--;

  if (!n.args.empty()) {
    indent();
    out << "args:\n";

    depth++;

    for (auto &arg : n.args)
      print(*arg);

    depth--;
  }

  depth--;
}

void AstPrinter::visit(Table &n) {
  indent();
  out << "table\n";

  depth++;

  for (auto &field : n.fields)
    print(*field);

  depth--;
}

void AstPrinter::visit(TableField &n) {
  indent();
  out << "table field\n";

  depth++;

  indent();
  out << "key:\n";

  depth++;
  print(*n.key);
  depth--;

  indent();
  out << "val:\n";

  depth++;
  print(*n.value);
  depth--;

  depth--;
}

void AstPrinter::visit(Attrib &n) {
  indent();
  out << "attrib(" << n.name << ")\n";
}

void AstPrinter::visit(FuncDef &n) {
  indent();

  out << "funcbody(";

  for (size_t i = 0; i < n.params.size(); ++i) {
    out << n.params[i];

    if (i + 1 < n.params.size())
      out << ", ";
  }

  if (n.hasVararg) {
    if (!n.params.empty())
      out << ", ";

    out << "...";

    if (n.varargName)
      out << ' ' << *n.varargName;
  }

  out << ")\n";

  depth++;
  print(*n.body);
  depth--;
}

void AstPrinter::visit(Block &n) {
  indent();
  out << "block\n";

  depth++;

  for (auto &stmt : n.stmts)
    print(*stmt);

  depth--;
}

void AstPrinter::visit(Break &) {
  indent();
  out << "break\n";
}

void AstPrinter::visit(Goto &n) {
  indent();
  out << "goto " << n.label << '\n';
}

void AstPrinter::visit(Label &n) {
  indent();
  out << "::" << n.name << "::\n";
}

void AstPrinter::visit(Do &n) {
  indent();
  out << "do\n";

  depth++;
  print(*n.body);
  depth--;
}

void AstPrinter::visit(While &n) {
  indent();
  out << "while\n";

  depth++;

  indent();
  out << "cond:\n";

  depth++;
  print(*n.cond);
  depth--;

  indent();
  out << "body:\n";

  depth++;
  print(*n.body);
  depth--;

  depth--;
}

void AstPrinter::visit(Repeat &n) {
  indent();
  out << "repeat\n";

  depth++;

  indent();
  out << "body:\n";

  depth++;
  print(*n.body);
  depth--;

  indent();
  out << "until:\n";

  depth++;
  print(*n.cond);
  depth--;

  depth--;
}

void AstPrinter::visit(If &n) {
  indent();
  out << "if\n";

  depth++;

  for (size_t i = 0; i < n.branches.size(); ++i) {

    indent();
    out << (i == 0 ? "cond:" : "elseif:") << '\n';

    depth++;
    print(*n.branches[i].cond);
    depth--;

    indent();
    out << "body:\n";

    depth++;
    print(*n.branches[i].body);
    depth--;
  }

  if (n.elseBody) {
    indent();
    out << "else:\n";

    depth++;
    print(*n.elseBody);
    depth--;
  }

  depth--;
}

void AstPrinter::visit(ForNum &n) {
  indent();
  out << "for_num(" << n.name << ")\n";

  depth++;

  indent();
  out << "start:\n";

  depth++;
  print(*n.start);
  depth--;

  indent();
  out << "limit:\n";

  depth++;
  print(*n.limit);
  depth--;

  if (n.step) {
    indent();
    out << "step:\n";

    depth++;
    print(*n.step);
    depth--;
  }

  indent();
  out << "body:\n";

  depth++;
  print(*n.body);
  depth--;

  depth--;
}

void AstPrinter::visit(ForIn &n) {
  indent();

  out << "for_in(";

  for (size_t i = 0; i < n.names.size(); ++i) {
    out << n.names[i];

    if (i + 1 < n.names.size())
      out << ", ";
  }

  out << ")\n";

  depth++;

  indent();
  out << "iters:\n";

  depth++;

  for (auto &iter : n.iterators)
    print(*iter);

  depth--;

  indent();
  out << "body:\n";

  depth++;

  print(*n.body);

  depth--;

  depth--;
}

void AstPrinter::visit(Assign &n) {
  indent();
  out << "assign\n";

  depth++;

  indent();
  out << "targets:\n";

  depth++;

  for (auto &target : n.targets)
    print(*target);

  depth--;

  indent();
  out << "values:\n";

  depth++;

  for (auto &value : n.values)
    print(*value);

  depth--;

  depth--;
}

void AstPrinter::visit(Decl &n) {
  indent();
  out << "var" << '\n';

  depth++;

  indent();
  out << "names:\n";
  depth++;
  for (size_t i = 0; i < n.names.size(); ++i) {
    indent();

    auto name = n.names[i];
    out << name.name;
    if (name.attrib.has_value())
      out << " <" << name.attrib->name << ">";
    out << '\n';
  }
  depth--;

  if (!n.values.empty()) {
    indent();
    out << "values:\n";

    depth++;

    for (auto &value : n.values)
      print(*value);

    depth--;
  }

  depth--;
}

void AstPrinter::visit(Return &n) {
  indent();
  out << "return\n";

  depth++;

  for (auto &value : n.values)
    print(*value);

  depth--;
  ;
}

void AstPrinter::visit(Func &n) {
  indent();

  out << "function ";

  for (size_t i = 0; i < n.path.size(); ++i) {
    out << n.path[i];

    if (i + 1 < n.path.size())
      out << '.';
  }

  if (n.method)
    out << ':' << *n.method;

  out << '\n';

  depth++;
  print(*n.body);
  depth--;
}

void AstPrinter::visit(ExprStmt &n) { print(*n.expr); }

} // namespace sol::syntax::ast
