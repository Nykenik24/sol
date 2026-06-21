#include "common/types.h"
#include "slexer.h"
#include "snode.h"
#include "sparser.h"
#include <stdio.h>

static void indent(int depth) {
  for (int i = 0; i < depth; i++)
    fputs("  ", stdout);
}

static void print_node(node_t *node, int depth);

static void print_nodes(node_t **nodes, ulong n, int depth) {
  for (ulong i = 0; i < n; i++)
    print_node(nodes[i], depth);
}

static void print_node(node_t *node, int depth) {
  if (!node) {
    indent(depth);
    puts("(null)");
    return;
  }

  switch (node->kind) {
  case SOL_NODE_BLOCK:
    indent(depth);
    puts("block");
    print_nodes(node->u.block.stmts, node->u.block.n, depth + 1);
    if (node->u.block.retstat)
      print_node(node->u.block.retstat, depth + 1);
    break;

  case SOL_NODE_DO:
    indent(depth);
    puts("do");
    print_nodes(node->u.block.stmts, node->u.block.n, depth + 1);
    break;

  case SOL_NODE_BREAK:
    indent(depth);
    puts("break");
    break;

  case SOL_NODE_RETURN:
    indent(depth);
    puts("return");
    print_nodes(node->u.ret.explist, node->u.ret.n, depth + 1);
    break;

  case SOL_NODE_GOTO:
    indent(depth);
    printf("goto %s\n", node->u.str);
    break;

  case SOL_NODE_LABEL:
    indent(depth);
    printf("::%s::\n", node->u.str);
    break;

  case SOL_NODE_NIL:
    indent(depth);
    puts("nil");
    break;
  case SOL_NODE_TRUE:
    indent(depth);
    puts("true");
    break;
  case SOL_NODE_FALSE:
    indent(depth);
    puts("false");
    break;
  case SOL_NODE_VARARG:
    indent(depth);
    puts("...");
    break;

  case SOL_NODE_INT:
    indent(depth);
    printf("digit(%g)\n", node->u.num);
    break;

  case SOL_NODE_HEX_DIGIT:
    indent(depth);
    printf("hex(%g)\n", node->u.num);
    break;

  case SOL_NODE_STRING:
    indent(depth);
    printf("string(%s)\n", node->u.str);
    break;

  case SOL_NODE_IDENT:
    indent(depth);
    printf("ident(%s)\n", node->u.str);
    break;

  case SOL_NODE_RANGE:
    indent(depth);
    printf("range(%g%s%g)\n", node->u.range.start,
           node->u.range.inclusive ? "..." : "..", node->u.range.end);
    break;
  case SOL_NODE_BINOP:
    indent(depth);
    printf("binop(%s)\n", node->u.binop.op->txt);
    print_node(node->u.binop.left, depth + 1);
    print_node(node->u.binop.right, depth + 1);
    break;

  case SOL_NODE_UNOP:
    indent(depth);
    printf("unop(%s)\n", node->u.unop.op->txt);
    print_node(node->u.unop.operand, depth + 1);
    break;

  case SOL_NODE_WHILE:
    indent(depth);
    puts("while");
    indent(depth + 1);
    puts("cond:");
    print_node(node->u.while_loop.cond, depth + 2);
    indent(depth + 1);
    puts("body:");
    print_node(node->u.while_loop.body, depth + 2);
    break;

  case SOL_NODE_REPEAT:
    indent(depth);
    puts("repeat");
    indent(depth + 1);
    puts("body:");
    print_node(node->u.repeat_loop.body, depth + 2);
    indent(depth + 1);
    puts("until:");
    print_node(node->u.repeat_loop.cond, depth + 2);
    break;

  case SOL_NODE_IF:
    indent(depth);
    puts("if");
    for (ulong i = 0; i < node->u.if_stmt.n; i++) {
      indent(depth + 1);
      printf("%s:\n", i == 0 ? "cond" : "elseif");
      print_node(node->u.if_stmt.conds[i], depth + 2);
      indent(depth + 1);
      puts("body:");
      print_node(node->u.if_stmt.bodies[i], depth + 2);
    }
    if (node->u.if_stmt.else_body) {
      indent(depth + 1);
      puts("else:");
      print_node(node->u.if_stmt.else_body, depth + 2);
    }
    break;

  case SOL_NODE_EACH:
    indent(depth);
    fputs("each(", stdout);

    for (ulong i = 0; i < node->u.each.names_n; i++) {
      fputs(node->u.each.names[i], stdout);

      if (i + 1 < node->u.each.names_n)
        fputs(", ", stdout);
    }

    puts(")");

    indent(depth + 1);
    puts("iter:");
    print_node(node->u.each.iter, depth + 2);

    indent(depth + 1);
    puts("body:");
    print_node(node->u.each.body, depth + 2);

    break;

  case SOL_NODE_ASSIGN:
    indent(depth);
    puts("assign");
    indent(depth + 1);
    puts("targets:");
    print_nodes(node->u.assign.targets, node->u.assign.target_n, depth + 2);
    indent(depth + 1);
    puts("values:");
    print_nodes(node->u.assign.values, node->u.assign.value_n, depth + 2);
    break;

  case SOL_NODE_LOCAL:
  case SOL_NODE_DECL:
    indent(depth);
    printf("var\n");
    indent(depth + 1);
    puts("names:");
    for (ulong i = 0; i < node->u.decl.n; i++) {
      indent(depth + 2);
      if (node->u.decl.attribs && node->u.decl.attribs[i])
        printf("%s <%s>\n", node->u.decl.names[i], node->u.decl.attribs[i]);
      else
        printf("%s\n", node->u.decl.names[i]);
    }
    if (node->u.decl.value_n > 0) {
      indent(depth + 1);
      puts("values:");
      print_nodes(node->u.decl.values, node->u.decl.value_n, depth + 2);
    }
    break;

  case SOL_NODE_FUNC:
  case SOL_NODE_LOCAL_FUNC:
    indent(depth);
    if (node->kind == SOL_NODE_LOCAL_FUNC)
      fputs("local function ", stdout);
    else
      fputs("function ", stdout);
    for (ulong i = 0; i < node->u.func.path_n; i++) {
      fputs(node->u.func.path[i], stdout);
      if (i + 1 < node->u.func.path_n)
        fputs(".", stdout);
    }
    if (node->u.func.method)
      printf(":%s", node->u.func.method);
    puts("");
    print_node(node->u.func.body, depth + 1);
    break;

  case SOL_NODE_FUNC_DEF:
    indent(depth);
    fputs("funcbody(", stdout);
    for (ulong i = 0; i < node->u.funcbody.param_n; i++) {
      fputs(node->u.funcbody.params[i], stdout);
      if (i + 1 < node->u.funcbody.param_n)
        fputs(", ", stdout);
    }
    if (node->u.funcbody.has_vararg) {
      if (node->u.funcbody.param_n > 0)
        fputs(", ", stdout);
      fputs("...", stdout);
      if (node->u.funcbody.vararg_name)
        printf(" %s", node->u.funcbody.vararg_name);
    }
    puts(")");
    print_node(node->u.funcbody.body, depth + 1);
    break;

  case SOL_NODE_FUNC_CALL:
    indent(depth);
    puts("call");
    indent(depth + 1);
    puts("target:");
    print_node(node->u.call.target, depth + 2);
    if (node->u.call.arg_n > 0) {
      indent(depth + 1);
      puts("args:");
      print_nodes(node->u.call.args, node->u.call.arg_n, depth + 2);
    }
    break;

  case SOL_NODE_METHOD_CALL:
    indent(depth);
    printf("method_call(:%s)\n", node->u.method_call.method);
    indent(depth + 1);
    puts("target:");
    print_node(node->u.method_call.target, depth + 2);
    if (node->u.method_call.arg_n > 0) {
      indent(depth + 1);
      puts("args:");
      print_nodes(node->u.method_call.args, node->u.method_call.arg_n,
                  depth + 2);
    }
    break;

  case SOL_NODE_INDEX:
    indent(depth);
    puts("index");
    indent(depth + 1);
    puts("target:");
    print_node(node->u.index.target, depth + 2);
    indent(depth + 1);
    puts("key:");
    print_node(node->u.index.key, depth + 2);
    break;

  case SOL_NODE_FIELD:
    indent(depth);
    printf("field(.%s)\n", node->u.field.name);
    indent(depth + 1);
    puts("target:");
    print_node(node->u.field.target, depth + 2);
    break;

  case SOL_NODE_TABLE:
    indent(depth);
    puts("table");
    print_nodes(node->u.table.fields, node->u.table.n, depth + 1);
    break;

  case SOL_NODE_TABLE_FIELD:
    indent(depth);
    puts("table field");
    indent(depth + 1);
    puts("key:");
    print_node(node->u.table_field.key, depth + 2);
    indent(depth + 1);
    puts("val:");
    print_node(node->u.table_field.val, depth + 2);
    break;

  case SOL_NODE_ATTRIB:
    indent(depth);
    printf("attrib(%s <%s>)\n", node->u.attrib.name, node->u.attrib.attrib);
    break;

  default:
    indent(depth);
    printf("unknown_node(%d)\n", node->kind);
    break;
  }
}

void print_ast(node_t *root) { print_node(root, 0); }
