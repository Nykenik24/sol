#include "pluja/parser/node.h"
#include <stdlib.h>

void plj_node_destroy(node_t *node) {
  if (!node)
    return;

  switch (node->kind) {
  case PLJ_NODE_IDENT:
  case PLJ_NODE_STRING:
    free((void *)node->u.str);
    break;

  case PLJ_NODE_BINOP:
    plj_node_destroy(node->u.binop.left);
    plj_node_destroy(node->u.binop.right);
    break;

  case PLJ_NODE_UNOP:
    plj_node_destroy(node->u.unop.operand);
    break;

  case PLJ_NODE_BLOCK:
  case PLJ_NODE_DO:
    for (size i = 0; i < node->u.block.n; i++)
      plj_node_destroy(node->u.block.stmts[i]);
    free(node->u.block.stmts);
    plj_node_destroy(node->u.block.retstat);
    break;

  case PLJ_NODE_WHILE:
    plj_node_destroy(node->u.while_loop.cond);
    plj_node_destroy(node->u.while_loop.body);
    break;

  case PLJ_NODE_REPEAT:
    plj_node_destroy(node->u.repeat_loop.body);
    plj_node_destroy(node->u.repeat_loop.cond);
    break;

  case PLJ_NODE_IF:
    for (size i = 0; i < node->u.if_stmt.n; i++) {
      plj_node_destroy(node->u.if_stmt.conds[i]);
      plj_node_destroy(node->u.if_stmt.bodies[i]);
    }
    free(node->u.if_stmt.conds);
    free(node->u.if_stmt.bodies);
    plj_node_destroy(node->u.if_stmt.else_body);
    break;

  case PLJ_NODE_FOR_NUM:
    free((void *)node->u.for_num.name);
    plj_node_destroy(node->u.for_num.start);
    plj_node_destroy(node->u.for_num.limit);
    plj_node_destroy(node->u.for_num.step);
    plj_node_destroy(node->u.for_num.body);
    break;

  case PLJ_NODE_FOR_IN:
    for (size i = 0; i < node->u.for_in.name_n; i++)
      free((void *)node->u.for_in.names[i]);
    free(node->u.for_in.names);
    for (size i = 0; i < node->u.for_in.iter_n; i++)
      plj_node_destroy(node->u.for_in.iters[i]);
    free(node->u.for_in.iters);
    plj_node_destroy(node->u.for_in.body);
    break;

  case PLJ_NODE_RETURN:
    for (size i = 0; i < node->u.ret.n; i++)
      plj_node_destroy(node->u.ret.explist[i]);
    free(node->u.ret.explist);
    break;

  case PLJ_NODE_ASSIGN:
    for (size i = 0; i < node->u.assign.target_n; i++)
      plj_node_destroy(node->u.assign.targets[i]);
    free(node->u.assign.targets);
    for (size i = 0; i < node->u.assign.value_n; i++)
      plj_node_destroy(node->u.assign.values[i]);
    free(node->u.assign.values);
    break;

  case PLJ_NODE_LOCAL:
  case PLJ_NODE_GLOBAL:
  case PLJ_NODE_DECL:
    for (size i = 0; i < node->u.decl.n; i++) {
      free((void *)node->u.decl.names[i]);
      if (node->u.decl.attribs)
        free((void *)node->u.decl.attribs[i]);
    }
    free(node->u.decl.names);
    free(node->u.decl.attribs);
    for (size i = 0; i < node->u.decl.value_n; i++)
      plj_node_destroy(node->u.decl.values[i]);
    free(node->u.decl.values);
    break;

  case PLJ_NODE_FUNC:
  case PLJ_NODE_LOCAL_FUNC:
    for (size i = 0; i < node->u.func.path_n; i++)
      free((void *)node->u.func.path[i]);
    free(node->u.func.path);
    free((void *)node->u.func.method);
    plj_node_destroy(node->u.func.body);
    break;

  case PLJ_NODE_FUNC_DEF:
    for (size i = 0; i < node->u.funcbody.param_n; i++)
      free((void *)node->u.funcbody.params[i]);
    free(node->u.funcbody.params);
    free((void *)node->u.funcbody.vararg_name);
    plj_node_destroy(node->u.funcbody.body);
    break;

  case PLJ_NODE_FUNC_CALL:
    plj_node_destroy(node->u.call.target);
    for (size i = 0; i < node->u.call.arg_n; i++)
      plj_node_destroy(node->u.call.args[i]);
    free(node->u.call.args);
    break;

  case PLJ_NODE_METHOD_CALL:
    plj_node_destroy(node->u.method_call.target);
    free((void *)node->u.method_call.method);
    for (size i = 0; i < node->u.method_call.arg_n; i++)
      plj_node_destroy(node->u.method_call.args[i]);
    free(node->u.method_call.args);
    break;

  case PLJ_NODE_INDEX:
    plj_node_destroy(node->u.index.target);
    plj_node_destroy(node->u.index.key);
    break;

  case PLJ_NODE_FIELD:
    plj_node_destroy(node->u.field.target);
    free((void *)node->u.field.name);
    break;

  case PLJ_NODE_TABLE:
    for (size i = 0; i < node->u.table.n; i++)
      plj_node_destroy(node->u.table.fields[i]);
    free(node->u.table.fields);
    break;

  case PLJ_NODE_TABLE_FIELD:
    plj_node_destroy(node->u.table_field.key);
    plj_node_destroy(node->u.table_field.val);
    break;

  case PLJ_NODE_ATTRIB:
    free((void *)node->u.attrib.name);
    free((void *)node->u.attrib.attrib);
    break;

  case PLJ_NODE_GOTO:
  case PLJ_NODE_LABEL:
    free((void *)node->u.str);
    break;

  case PLJ_NODE_DIGIT:
  case PLJ_NODE_HEX_DIGIT:
  case PLJ_NODE_NIL:
  case PLJ_NODE_TRUE:
  case PLJ_NODE_FALSE:
  case PLJ_NODE_VARARG:
  case PLJ_NODE_BREAK:
  case PLJ_NODE_GLOBAL_WILDCARD:
    break;
  }

  free(node);
}
