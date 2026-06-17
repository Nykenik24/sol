#include "sol/parser/node.h"
#include <stdlib.h>

void sol_node_destroy(Node *node) {
  if (!node)
    return;

  switch (node->kind) {
  case SOL_NODE_IDENT:
  case SOL_NODE_STRING:
    free((void *)node->u.str);
    break;

  case SOL_NODE_BINOP:
    sol_node_destroy(node->u.binop.left);
    sol_node_destroy(node->u.binop.right);
    break;

  case SOL_NODE_UNOP:
    sol_node_destroy(node->u.unop.operand);
    break;

  case SOL_NODE_BLOCK:
  case SOL_NODE_DO:
    for (size i = 0; i < node->u.block.n; i++)
      sol_node_destroy(node->u.block.stmts[i]);
    free(node->u.block.stmts);
    sol_node_destroy(node->u.block.retstat);
    break;

  case SOL_NODE_WHILE:
    sol_node_destroy(node->u.while_loop.cond);
    sol_node_destroy(node->u.while_loop.body);
    break;

  case SOL_NODE_REPEAT:
    sol_node_destroy(node->u.repeat_loop.body);
    sol_node_destroy(node->u.repeat_loop.cond);
    break;

  case SOL_NODE_IF:
    for (size i = 0; i < node->u.if_stmt.n; i++) {
      sol_node_destroy(node->u.if_stmt.conds[i]);
      sol_node_destroy(node->u.if_stmt.bodies[i]);
    }
    free(node->u.if_stmt.conds);
    free(node->u.if_stmt.bodies);
    sol_node_destroy(node->u.if_stmt.else_body);
    break;

  case SOL_NODE_FOR_NUM:
    free((void *)node->u.for_num.name);
    sol_node_destroy(node->u.for_num.start);
    sol_node_destroy(node->u.for_num.limit);
    sol_node_destroy(node->u.for_num.step);
    sol_node_destroy(node->u.for_num.body);
    break;

  case SOL_NODE_FOR_IN:
    for (size i = 0; i < node->u.for_in.name_n; i++)
      free((void *)node->u.for_in.names[i]);
    free(node->u.for_in.names);
    for (size i = 0; i < node->u.for_in.iter_n; i++)
      sol_node_destroy(node->u.for_in.iters[i]);
    free(node->u.for_in.iters);
    sol_node_destroy(node->u.for_in.body);
    break;

  case SOL_NODE_RETURN:
    for (size i = 0; i < node->u.ret.n; i++)
      sol_node_destroy(node->u.ret.explist[i]);
    free(node->u.ret.explist);
    break;

  case SOL_NODE_ASSIGN:
    for (size i = 0; i < node->u.assign.target_n; i++)
      sol_node_destroy(node->u.assign.targets[i]);
    free(node->u.assign.targets);
    for (size i = 0; i < node->u.assign.value_n; i++)
      sol_node_destroy(node->u.assign.values[i]);
    free(node->u.assign.values);
    break;

  case SOL_NODE_LOCAL:
  case SOL_NODE_GLOBAL:
  case SOL_NODE_DECL:
    for (size i = 0; i < node->u.decl.n; i++) {
      free((void *)node->u.decl.names[i]);
      if (node->u.decl.attribs)
        free((void *)node->u.decl.attribs[i]);
    }
    free(node->u.decl.names);
    free(node->u.decl.attribs);
    for (size i = 0; i < node->u.decl.value_n; i++)
      sol_node_destroy(node->u.decl.values[i]);
    free(node->u.decl.values);
    break;

  case SOL_NODE_FUNC:
  case SOL_NODE_LOCAL_FUNC:
    for (size i = 0; i < node->u.func.path_n; i++)
      free((void *)node->u.func.path[i]);
    free(node->u.func.path);
    free((void *)node->u.func.method);
    sol_node_destroy(node->u.func.body);
    break;

  case SOL_NODE_FUNC_DEF:
    for (size i = 0; i < node->u.funcbody.param_n; i++)
      free((void *)node->u.funcbody.params[i]);
    free(node->u.funcbody.params);
    free((void *)node->u.funcbody.vararg_name);
    sol_node_destroy(node->u.funcbody.body);
    break;

  case SOL_NODE_FUNC_CALL:
    sol_node_destroy(node->u.call.target);
    for (size i = 0; i < node->u.call.arg_n; i++)
      sol_node_destroy(node->u.call.args[i]);
    free(node->u.call.args);
    break;

  case SOL_NODE_METHOD_CALL:
    sol_node_destroy(node->u.method_call.target);
    free((void *)node->u.method_call.method);
    for (size i = 0; i < node->u.method_call.arg_n; i++)
      sol_node_destroy(node->u.method_call.args[i]);
    free(node->u.method_call.args);
    break;

  case SOL_NODE_INDEX:
    sol_node_destroy(node->u.index.target);
    sol_node_destroy(node->u.index.key);
    break;

  case SOL_NODE_FIELD:
    sol_node_destroy(node->u.field.target);
    free((void *)node->u.field.name);
    break;

  case SOL_NODE_TABLE:
    for (size i = 0; i < node->u.table.n; i++)
      sol_node_destroy(node->u.table.fields[i]);
    free(node->u.table.fields);
    break;

  case SOL_NODE_TABLE_FIELD:
    sol_node_destroy(node->u.table_field.key);
    sol_node_destroy(node->u.table_field.val);
    break;

  case SOL_NODE_ATTRIB:
    free((void *)node->u.attrib.name);
    free((void *)node->u.attrib.attrib);
    break;

  case SOL_NODE_GOTO:
  case SOL_NODE_LABEL:
    free((void *)node->u.str);
    break;

  case SOL_NODE_DIGIT:
  case SOL_NODE_HEX_DIGIT:
  case SOL_NODE_NIL:
  case SOL_NODE_TRUE:
  case SOL_NODE_FALSE:
  case SOL_NODE_VARARG:
  case SOL_NODE_BREAK:
    break;
  }

  free(node);
}
