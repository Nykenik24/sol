#include "sol/vm/vm.h"
#include "sol/error.h"
#include "sol/lexer/token.h"
#include "sol/parser/node.h"
#include "sol/types.h"
#include "vmnatives.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *sol_vtype_to_str(ValueType vt) {
#define CASE(VT, STR)                                                          \
  case VT:                                                                     \
    return STR
  switch (vt) {
    CASE(VAL_NUMBER, "number");
    CASE(VAL_STRING, "string");
    CASE(VAL_BOOLEAN, "boolean");
    CASE(VAL_NIL, "nil");
    CASE(VAL_NATIVE, "native function");
  }
#undef CASE
}

char *sol_val_to_str(Value v) {
  switch (v.type) {
  case VAL_NUMBER: {
    char str[4096];
    sprintf(str, "%g", v.as.num);
    return strdup(str);
  }
  case VAL_STRING:
    return (char *)v.as.str;
  case VAL_BOOLEAN:
    return v.as.b ? "true" : "false";
  case VAL_NIL:
    return "nil";
  case VAL_NATIVE:
    return "<native_function>";
  }
}

Value make_nil() { return (Value){.type = VAL_NIL}; }

Value make_number(double n) { return (Value){.type = VAL_NUMBER, .as.num = n}; }

Value make_string(const char *s) {
  return (Value){.type = VAL_STRING, .as.str = s};
}

Value make_bool(bool b) { return (Value){.type = VAL_BOOLEAN, .as.b = b}; }

Value make_native(Fn fn) {
  return (Value){.type = VAL_NATIVE, .as.native.fn = fn};
}

Env *env_create(Env *parent) {
  Env *e = malloc(sizeof(Env));
  e->parent = parent;
  e->names = NULL;
  e->values = NULL;
  e->count = 0;
  e->capacity = 0;
  return e;
}

void env_destroy(Env *e) {
  if (!e)
    return;

  free(e->names);
  free(e->values);

  env_destroy(e->parent);
  free(e);
}

void env_set(Env *e, const char *name, Value v) {
  for (Env *cur = e; cur; cur = cur->parent) {
    for (size i = 0; i < cur->count; i++) {
      if (strcmp(cur->names[i], name) == 0) {
        cur->values[i] = v;
        return;
      }
    }
  }

  if (e->count == e->capacity) {
    e->capacity = e->capacity ? e->capacity * 2 : 8;

    e->names = realloc(e->names, sizeof(char *) * e->capacity);
    e->values = realloc(e->values, sizeof(Value) * e->capacity);
  }

  e->names[e->count] = name;
  e->values[e->count] = v;
  e->count++;
}

Value env_get(Env *e, const char *name) {
  for (Env *cur = e; cur; cur = cur->parent) {
    for (size i = 0; i < cur->count; i++) {
      if (strcmp(cur->names[i], name) == 0) {
        return cur->values[i];
      }
    }
  }

  return make_nil();
}

#include <stdio.h>

VM *sol_vm_create(void) {
  VM *vm = malloc(sizeof(VM));
  vm->env = env_create(NULL);

  env_set(vm->env, "print", make_native(native_print));

  return vm;
}

void sol_vm_destroy(VM *vm) {
  if (!vm)
    return;

  if (vm->env)
    env_destroy(vm->env);

  free(vm);
}

Value eval_node(VM *vm, Node *n);
void exec_block(VM *vm, Node *n);

void sol_vm_exec(VM *vm, Node **nodes, size n) {
  for (size i = 0; i < n; i++) {
    eval_node(vm, nodes[i]);
  }
}

static bool val_as_bool(Value val) {
  switch (val.type) {
  case VAL_NUMBER:
  case VAL_STRING:
  case VAL_NATIVE:
    return true;
  case VAL_BOOLEAN:
    return val.as.b;
  default:
    return false;
  }
}

static Value eval_binop(VM *vm, Node *n) {
  Value l = eval_node(vm, n->u.binop.left);
  Value r = eval_node(vm, n->u.binop.right);

  if (l.type == VAL_NUMBER && r.type == VAL_NUMBER) {
    double ln = l.as.num;
    double rn = r.as.num;
    switch (n->u.binop.op->type) {
    case SOL_TK_SYM_ADD:
      return make_number(ln + rn);
    case SOL_TK_SYM_SUB:
      return make_number(ln - rn);
    case SOL_TK_SYM_MUL:
      return make_number(ln * rn);
    case SOL_TK_SYM_DIV:
      return make_number(ln / rn);
    case SOL_TK_SYM_MOD:
      return make_number((int)ln % (int)rn);
    case SOL_TK_SYM_BITAND:
      return make_number((uint)ln & (uint)rn);
    case SOL_TK_SYM_BITOR:
      return make_number((uint)ln | (uint)rn);
    case SOL_TK_SYM_BITXOR:
      return make_number((uint)ln ^ (uint)rn);
    case SOL_TK_SYM_LEFTSHIFT:
      return make_number((uint)ln << (uint)rn);
    case SOL_TK_SYM_RIGHTSHIFT:
      return make_number((uint)ln >> (uint)rn);
    case SOL_TK_SYM_EQUAL:
      return make_bool(ln == rn);
    case SOL_TK_SYM_NEQUAL:
      return make_bool(ln != rn);
    case SOL_TK_SYM_MORE:
      return make_bool(ln > rn);
    case SOL_TK_SYM_LESS:
      return make_bool(ln < rn);
    case SOL_TK_SYM_GEQUAL:
      return make_bool(ln >= rn);
    case SOL_TK_SYM_LEQUAL:
      return make_bool(ln <= rn);
    default:
      break;
    }
  } else if (l.type == VAL_STRING && r.type == VAL_STRING) {
    const char *ls = l.as.str;
    const char *rs = r.as.str;
    switch (n->u.binop.op->type) {
    case SOL_TK_SYM_EQUAL: {
      return make_bool(strcmp(ls, rs) == 0);
    }
    case SOL_TK_SYM_NEQUAL:
      return make_bool(strcmp(ls, rs) != 0);
    default:
      break;
    }
  } else {
    switch (n->u.binop.op->type) {
    case SOL_TK_KW_AND: {
      return make_bool(val_as_bool(l) && val_as_bool(r));
    case SOL_TK_KW_OR:
      return make_bool(val_as_bool(l) || val_as_bool(r));
    }
    default:
      break;
    }
  }

  sol_fatal("unsupported binary operator '%s' for %s and %s\n",
            n->u.binop.op->txt, sol_vtype_to_str(l.type),
            sol_vtype_to_str(r.type));
  return make_nil();
}

static Value eval_unop(VM *vm, Node *n) {
  Value v = eval_node(vm, n->u.unop.operand);

  switch (n->u.unop.op->type) {
  case SOL_TK_SYM_SUB:
    if (v.type == VAL_NUMBER)
      return make_number(-v.as.num);
    break;
  case SOL_TK_SYM_BITNOT:
    if (v.type == VAL_NUMBER)
      return make_number(~(uint)v.as.num);
  default:
    break;
  }

  sol_fatal("unsupported unary operator '%s for %s\n", n->u.unop.op->txt,
            sol_vtype_to_str(v.type));
  return make_nil();
}

static Value eval_ident(VM *vm, Node *n) { return env_get(vm->env, n->u.str); }

static Value eval_number(Node *n) { return make_number(n->u.num); }

static Value eval_call(VM *vm, Node *n) {
  Value target = eval_node(vm, n->u.call.target);

  int cap = 16;
  int argn = 0;
  Value *args = calloc(cap, sizeof(Value));
  for (size i = 0; i < n->u.call.arg_n; i++) {
    if (argn > cap) {
      cap *= 2;
      args = realloc(args, sizeof(Value) * cap);
    }
    args[argn++] = eval_node(vm, n->u.call.args[i]);
  }

  if (target.type == VAL_NATIVE) {
    return target.as.native.fn(vm, args, n->u.call.arg_n);
  }

  return make_nil();
}

static Value exec_assign(VM *vm, Node *n) {
  for (size i = 0; i < n->u.assign.target_n; i++) {
    Node *t = n->u.assign.targets[i];
    Value v = eval_node(vm, n->u.assign.values[i]);

    if (t->kind == SOL_NODE_IDENT) {
      env_set(vm->env, t->u.str, v);
    }
  }

  return make_nil();
}

static Value exec_decl(VM *vm, Node *n) {
  for (size i = 0; i < n->u.decl.n; i++) {
    Value v = eval_node(vm, n->u.decl.values[i]);

    env_set(vm->env, n->u.decl.names[i], v);
  }

  if (n->u.decl.n == 1)
    return eval_node(vm, n->u.decl.values[0]);
  return make_nil();
}

void exec_block(VM *vm, Node *n) {
  Env *prev = vm->env;
  vm->env = env_create(prev);

  for (size i = 0; i < n->u.block.n; i++) {
    eval_node(vm, n->u.block.stmts[i]);
  }

  vm->env = prev;
}

static void exec_if(VM *vm, Node *n) {
  for (size i = 0; i < n->u.if_stmt.n; i++) {
    Value cond = eval_node(vm, n->u.if_stmt.conds[i]);

    if (cond.type == VAL_BOOLEAN && cond.as.b == true) {
      exec_block(vm, n->u.if_stmt.bodies[i]);
      return;
    }
  }

  if (n->u.if_stmt.else_body) {
    exec_block(vm, n->u.if_stmt.else_body);
  }
}

static void exec_while(VM *vm, Node *n) {
  while (1) {
    Value cond = eval_node(vm, n->u.while_loop.cond);

    if (cond.type != VAL_BOOLEAN || !cond.as.b)
      break;

    exec_block(vm, n->u.while_loop.body);
  }
}

static void exec_fornum(VM *vm, Node *n) {
  Value start = eval_node(vm, n->u.for_num.start);
  if (start.type != VAL_NUMBER)
    sol_fatal("for start must be a number\n");
  Value limit = eval_node(vm, n->u.for_num.limit);
  if (limit.type != VAL_NUMBER)
    sol_fatal("for limit must be a number\n");
  Value step = eval_node(vm, n->u.for_num.step);
  if (step.type != VAL_NUMBER)
    sol_fatal("for step must be a number\n");

  for (size i = start.as.num; i <= limit.as.num; i += step.as.num) {
    env_set(vm->env, n->u.for_num.name, make_number(i));
    exec_block(vm, n->u.for_num.body);
  }
}

Value eval_node(VM *vm, Node *n) {
  switch (n->kind) {

  case SOL_NODE_DIGIT:
    return make_number(n->u.num);

  case SOL_NODE_STRING:
    return make_string(n->u.str);

  case SOL_NODE_TRUE:
  case SOL_NODE_FALSE:
    return make_bool(n->kind == SOL_NODE_TRUE ? true : false);

  case SOL_NODE_IDENT:
    return eval_ident(vm, n);

  case SOL_NODE_BINOP:
    return eval_binop(vm, n);

  case SOL_NODE_UNOP:
    return eval_unop(vm, n);

  case SOL_NODE_LOCAL:
  case SOL_NODE_DECL:
    return exec_decl(vm, n);

  case SOL_NODE_ASSIGN:
    return exec_assign(vm, n);

  case SOL_NODE_BLOCK:
  case SOL_NODE_DO:
    exec_block(vm, n);
    return make_nil();

  case SOL_NODE_IF:
    exec_if(vm, n);
    return make_nil();

  case SOL_NODE_WHILE:
    exec_while(vm, n);
    return make_nil();

  case SOL_NODE_FOR_NUM:
    exec_fornum(vm, n);
    return make_nil();

  case SOL_NODE_FUNC_CALL:
    return eval_call(vm, n);

  default:
    return make_nil();
  }
}
