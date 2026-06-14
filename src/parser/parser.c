#include "sol/parser/parser.h"
#include "sol/error.h"
#include "sol/lexer/token.h"
#include "sol/mem.h"
#include "sol/parser/node.h"
#include "sol/types.h"
#include <stdlib.h>
#include <string.h>

void sol_cleanup_parse_res(node_t **nodes, uint64 node_num) {
  for (size i = 0; i < node_num; i++) {
    sol_node_destroy(nodes[i]);
  }
}

#define skip() (*i)++
#define node_alloc(N) node_t *N = malloc(sizeof(node_t))
#define match(TXT) (*i < tk_num && strcmp(tokens[*i]->txt, TXT) == 0)
#define match_type(T) (*i < tk_num && tokens[*i]->type == (T))
#define cur_txt() (tokens[*i]->txt)
#define cur_type() (tokens[*i]->type)
#define at_end() (*i >= tk_num || tokens[*i]->type == SOL_TK_EOF)

static node_t *p_exp(token_t **tokens, size *i, size tk_num);
static node_t *p_block(token_t **tokens, size *i, size tk_num);
static node_t *p_stmt(token_t **tokens, size *i, size tk_num);
static node_t *p_prefixexp(token_t **tokens, size *i, size tk_num);
static node_t *p_functioncall(token_t **tokens, size *i, size tk_num);

static const char *dup_str(const char *s) {
  size_t len = strlen(s);
  char *out = malloc(len + 1);
  memcpy(out, s, len + 1);
  return out;
}

static int is_binop(const char *txt) {
  static const char *ops[] = {"+",  "-",  "*",  "/",   "//", "^", "%",  "&",
                              "~",  "|",  ">>", "<<",  "..", "<", "<=", ">",
                              ">=", "==", "~=", "and", "or", NULL};
  for (int j = 0; ops[j]; j++)
    if (strcmp(txt, ops[j]) == 0)
      return 1;
  return 0;
}

static int is_unop(const char *txt) {
  return strcmp(txt, "-") == 0 || strcmp(txt, "not") == 0 ||
         strcmp(txt, "#") == 0 || strcmp(txt, "~") == 0;
}

static void expect(token_t **tokens, size *i, size tk_num, const char *txt) {
  if (at_end() || !match(txt))
    sol_fatal("expected '%s '%s' at line %lu\n'", txt, cur_txt(),
              tokens[*i]->line);
  skip();
}

static const char *expect_name(token_t **tokens, size *i, size tk_num) {
  if (at_end() || cur_type() != SOL_TK_IDENT)
    sol_fatal("expected name '%s' at line %lu\n", cur_txt(), tokens[*i]->line);
  const char *name = dup_str(cur_txt());
  skip();
  return name;
}

static const char *p_attrib(token_t **tokens, size *i, size tk_num) {
  if (!match("<"))
    return NULL;
  skip();
  const char *name = expect_name(tokens, i, tk_num);
  expect(tokens, i, tk_num, ">");
  return name;
}

static node_t *p_explist_into(token_t **tokens, size *i, size tk_num,
                              node_t ***out, size *out_n) {
  list_t *list = sol_list_init();
  sol_list_push(list, p_exp(tokens, i, tk_num));
  while (match(",")) {
    skip();
    sol_list_push(list, p_exp(tokens, i, tk_num));
  }
  *out = (node_t **)list->raw;
  *out_n = list->num;
  return NULL;
}

static void p_namelist_into(token_t **tokens, size *i, size tk_num,
                            const char ***out, size *out_n) {
  list_t *list = sol_list_init();
  sol_list_push(list, (void *)expect_name(tokens, i, tk_num));
  while (match(",") && tokens[*i + 1]->type == SOL_TK_IDENT) {
    skip();
    sol_list_push(list, (void *)expect_name(tokens, i, tk_num));
  }
  *out = (const char **)list->raw;
  *out_n = list->num;
}

static node_t *p_funcbody(token_t **tokens, size *i, size tk_num) {
  expect(tokens, i, tk_num, "(");

  list_t *params = sol_list_init();
  int has_vararg = 0;
  const char *vararg_name = NULL;

  if (!match(")")) {
    if (match("...")) {
      has_vararg = 1;
      skip();
      if (match_type(SOL_TK_IDENT)) {
        vararg_name = dup_str(cur_txt());
        skip();
      }
    } else {
      sol_list_push(params, (void *)expect_name(tokens, i, tk_num));
      while (match(",")) {
        skip();
        if (match("...")) {
          has_vararg = 1;
          skip();
          if (match_type(SOL_TK_IDENT)) {
            vararg_name = dup_str(cur_txt());
            skip();
          }
          break;
        }
        sol_list_push(params, (void *)expect_name(tokens, i, tk_num));
      }
    }
  }

  expect(tokens, i, tk_num, ")");
  node_t *body = p_block(tokens, i, tk_num);
  expect(tokens, i, tk_num, "end");

  node_alloc(node);
  node->kind = SOL_NODE_FUNC_DEF;
  node->u.funcbody.params = (node_t **)params->raw;
  node->u.funcbody.param_n = params->num;
  node->u.funcbody.has_vararg = has_vararg;
  node->u.funcbody.vararg_name = vararg_name;
  node->u.funcbody.body = body;
  return node;
}

static node_t *p_table(token_t **tokens, size *i, size tk_num) {
  expect(tokens, i, tk_num, "{");
  list_t *fields = sol_list_init();

  while (!match("}")) {
    node_alloc(field);

    if (match("[")) {
      skip();
      node_t *key = p_exp(tokens, i, tk_num);
      expect(tokens, i, tk_num, "]");
      expect(tokens, i, tk_num, "=");
      node_t *val = p_exp(tokens, i, tk_num);
      field->kind = SOL_NODE_TABLE_FIELD;
      field->u.table_field.key = key;
      field->u.table_field.val = val;
    } else if (cur_type() == SOL_TK_IDENT && !at_end() &&
               strcmp(tokens[*i + 1]->txt, "=") == 0) {
      const char *name = dup_str(cur_txt());
      skip();
      skip(); // '='
      node_t *val = p_exp(tokens, i, tk_num);
      field->kind = SOL_NODE_FIELD;
      field->u.field.name = name;
      field->u.field.target = val;
    } else {
      node_t *val = p_exp(tokens, i, tk_num);
      *field = *val;
      free(val);
    }

    sol_list_push(fields, field);

    if (match(",") || match(";"))
      skip();
    else
      break;
  }

  expect(tokens, i, tk_num, "}");

  node_alloc(node);
  node->kind = SOL_NODE_TABLE;
  node->u.table.fields = (node_t **)fields->raw;
  node->u.table.n = fields->num;
  return node;
}

static node_t **p_args(token_t **tokens, size *i, size tk_num, size *arg_n) {
  list_t *args = sol_list_init();

  if (match("(")) {
    skip();
    if (!match(")")) {
      sol_list_push(args, p_exp(tokens, i, tk_num));
      while (match(",")) {
        skip();
        sol_list_push(args, p_exp(tokens, i, tk_num));
      }
    }
    expect(tokens, i, tk_num, ")");
  } else if (match("{")) {
    sol_list_push(args, p_table(tokens, i, tk_num));
  } else if (cur_type() == SOL_TK_STRING) {
    node_alloc(s);
    s->kind = SOL_NODE_STRING;
    s->u.str = dup_str(cur_txt());
    skip();
    sol_list_push(args, s);
  } else {
    sol_fatal("expected function arguments '%s'\n", cur_txt());
  }

  *arg_n = args->num;
  return (node_t **)args->raw;
}

static node_t *p_primaryexp(token_t **tokens, size *i, size tk_num) {
  if (cur_type() == SOL_TK_IDENT) {
    node_alloc(node);
    node->kind = SOL_NODE_IDENT;
    node->u.str = dup_str(cur_txt());
    skip();
    return node;
  }

  if (match("(")) {
    skip();
    node_t *inner = p_exp(tokens, i, tk_num);
    expect(tokens, i, tk_num, ")");
    return inner;
  }

  sol_fatal("unexpected symbol '%s' at line %lu\n", cur_txt(),
            tokens[*i]->line);
  return NULL;
}

static node_t *p_prefixexp(token_t **tokens, size *i, size tk_num) {
  node_t *node = p_primaryexp(tokens, i, tk_num);

  while (!at_end()) {
    if (match(".")) {
      skip();
      const char *name = expect_name(tokens, i, tk_num);
      node_alloc(field);
      field->kind = SOL_NODE_FIELD;
      field->u.field.target = node;
      field->u.field.name = name;
      node = field;
    } else if (match("[")) {
      skip();
      node_t *key = p_exp(tokens, i, tk_num);
      expect(tokens, i, tk_num, "]");
      node_alloc(idx);
      idx->kind = SOL_NODE_INDEX;
      idx->u.index.target = node;
      idx->u.index.key = key;
      node = idx;
    } else if (match(":")) {
      skip();
      const char *method = expect_name(tokens, i, tk_num);
      size arg_n;
      node_t **args = p_args(tokens, i, tk_num, &arg_n);
      node_alloc(call);
      call->kind = SOL_NODE_METHOD_CALL;
      call->u.method_call.target = node;
      call->u.method_call.method = method;
      call->u.method_call.args = args;
      call->u.method_call.arg_n = arg_n;
      node = call;
    } else if (match("(") || match("{") || cur_type() == SOL_TK_STRING) {
      size arg_n;
      node_t **args = p_args(tokens, i, tk_num, &arg_n);
      node_alloc(call);
      call->kind = SOL_NODE_FUNC_CALL;
      call->u.call.target = node;
      call->u.call.args = args;
      call->u.call.arg_n = arg_n;
      node = call;
    } else {
      break;
    }
  }

  return node;
}

static node_t *p_simple_exp(token_t **tokens, size *i, size tk_num) {
  if (at_end())
    sol_fatal("unexpected end of input\n");

  if (match("nil")) {
    node_alloc(n);
    n->kind = SOL_NODE_NIL;
    skip();
    return n;
  }
  if (match("true")) {
    node_alloc(n);
    n->kind = SOL_NODE_TRUE;
    skip();
    return n;
  }
  if (match("false")) {
    node_alloc(n);
    n->kind = SOL_NODE_FALSE;
    skip();
    return n;
  }
  if (match("...")) {
    node_alloc(n);
    n->kind = SOL_NODE_VARARG;
    skip();
    return n;
  }
  if (cur_type() == SOL_TK_DIGIT) {
    node_alloc(n);
    n->kind = SOL_NODE_DIGIT;
    n->u.num = atof(cur_txt());
    skip();
    return n;
  }
  if (cur_type() == SOL_TK_HEX_DIGIT) {
    node_alloc(n);
    n->kind = SOL_NODE_HEX_DIGIT;
    n->u.num = (double)strtol(cur_txt(), NULL, 16);
    skip();
    return n;
  }
  if (cur_type() == SOL_TK_STRING) {
    node_alloc(n);
    n->kind = SOL_NODE_STRING;
    n->u.str = dup_str(cur_txt());
    skip();
    return n;
  }
  if (match("function")) {
    skip();
    return p_funcbody(tokens, i, tk_num);
  }
  if (match("{")) {
    return p_table(tokens, i, tk_num);
  }
  if (is_unop(cur_txt())) {
    token_t *op = tokens[*i];
    skip();
    node_t *operand = p_simple_exp(tokens, i, tk_num);
    node_alloc(n);
    n->kind = SOL_NODE_UNOP;
    n->u.unop.op = op;
    n->u.unop.operand = operand;
    return n;
  }

  return p_prefixexp(tokens, i, tk_num);
}

static int binop_precedence(const char *op) {
  if (strcmp(op, "or") == 0)
    return 1;
  if (strcmp(op, "and") == 0)
    return 2;
  if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || strcmp(op, "<=") == 0 ||
      strcmp(op, ">=") == 0 || strcmp(op, "==") == 0 || strcmp(op, "~=") == 0)
    return 3;
  if (strcmp(op, "|") == 0)
    return 4;
  if (strcmp(op, "~") == 0)
    return 5;
  if (strcmp(op, "&") == 0)
    return 6;
  if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0)
    return 7;
  if (strcmp(op, "..") == 0)
    return 8;
  if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
    return 9;
  if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "//") == 0 ||
      strcmp(op, "%") == 0)
    return 10;
  if (strcmp(op, "^") == 0)
    return 12;
  return -1;
}

static int is_right_assoc(const char *op) {
  return strcmp(op, "^") == 0 || strcmp(op, "..") == 0;
}

static node_t *p_exp_prec(token_t **tokens, size *i, size tk_num,
                          int min_prec) {
  node_t *left = p_simple_exp(tokens, i, tk_num);

  while (!at_end() && is_binop(cur_txt())) {
    const char *op_txt = cur_txt();
    int prec = binop_precedence(op_txt);
    if (prec < min_prec)
      break;

    token_t *op = tokens[*i];
    skip();

    int next_prec = is_right_assoc(op_txt) ? prec : prec + 1;
    node_t *right = p_exp_prec(tokens, i, tk_num, next_prec);

    node_alloc(bin);
    bin->kind = SOL_NODE_BINOP;
    bin->u.binop.left = left;
    bin->u.binop.right = right;
    bin->u.binop.op = op;
    left = bin;
  }

  return left;
}

static node_t *p_exp(token_t **tokens, size *i, size tk_num) {
  return p_exp_prec(tokens, i, tk_num, 1);
}

static node_t *p_block(token_t **tokens, size *i, size tk_num) {
  list_t *list = sol_list_init();
  node_t *retstat = NULL;

  while (!at_end() && !match("end") && !match("else") && !match("elseif") &&
         !match("until")) {
    if (match("return")) {
      skip();
      retstat = malloc(sizeof(node_t));
      retstat->kind = SOL_NODE_RETURN;
      list_t *rets = sol_list_init();
      if (!at_end() && !match(";") && !match("end") && !match("else") &&
          !match("elseif") && !match("until")) {
        sol_list_push(rets, p_exp(tokens, i, tk_num));
        while (match(",")) {
          skip();
          sol_list_push(rets, p_exp(tokens, i, tk_num));
        }
      }
      retstat->u.ret.explist = (node_t **)rets->raw;
      retstat->u.ret.n = rets->num;
      if (match(";"))
        skip();
      break;
    }

    node_t *s = p_stmt(tokens, i, tk_num);
    if (s)
      sol_list_push(list, s);
  }

  node_alloc(block);
  block->kind = SOL_NODE_BLOCK;
  block->u.block.stmts = (node_t **)list->raw;
  block->u.block.n = list->num;
  block->u.block.retstat = retstat;
  return block;
}

static node_t *p_stmt(token_t **tokens, size *i, size tk_num) {
  if (match(";")) {
    skip();
    return NULL;
  }

  if (match("break")) {
    node_alloc(n);
    n->kind = SOL_NODE_BREAK;
    skip();
    return n;
  }

  if (match("goto")) {
    skip();
    node_alloc(n);
    n->kind = SOL_NODE_GOTO;
    n->u.str = expect_name(tokens, i, tk_num);
    return n;
  }

  if (match("::")) {
    skip();
    const char *name = expect_name(tokens, i, tk_num);
    expect(tokens, i, tk_num, "::");
    node_alloc(n);
    n->kind = SOL_NODE_LABEL;
    n->u.str = name;
    return n;
  }

  if (match("do")) {
    skip();
    node_t *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "end");
    node_alloc(n);
    n->kind = SOL_NODE_DO;
    n->u.block = body->u.block;
    free(body);
    return n;
  }

  if (match("while")) {
    skip();
    node_t *cond = p_exp(tokens, i, tk_num);
    expect(tokens, i, tk_num, "do");
    node_t *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "end");
    node_alloc(n);
    n->kind = SOL_NODE_WHILE;
    n->u.while_loop.cond = cond;
    n->u.while_loop.body = body;
    return n;
  }

  if (match("repeat")) {
    skip();
    node_t *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "until");
    node_t *cond = p_exp(tokens, i, tk_num);
    node_alloc(n);
    n->kind = SOL_NODE_REPEAT;
    n->u.repeat_loop.body = body;
    n->u.repeat_loop.cond = cond;
    return n;
  }

  if (match("if")) {
    skip();
    list_t *conds = sol_list_init();
    list_t *bodies = sol_list_init();

    sol_list_push(conds, p_exp(tokens, i, tk_num));
    expect(tokens, i, tk_num, "then");
    sol_list_push(bodies, p_block(tokens, i, tk_num));

    while (match("elseif")) {
      skip();
      sol_list_push(conds, p_exp(tokens, i, tk_num));
      expect(tokens, i, tk_num, "then");
      sol_list_push(bodies, p_block(tokens, i, tk_num));
    }

    node_t *else_body = NULL;
    if (match("else")) {
      skip();
      else_body = p_block(tokens, i, tk_num);
    }

    expect(tokens, i, tk_num, "end");

    node_alloc(n);
    n->kind = SOL_NODE_IF;
    n->u.if_stmt.conds = (node_t **)conds->raw;
    n->u.if_stmt.bodies = (node_t **)bodies->raw;
    n->u.if_stmt.n = conds->num;
    n->u.if_stmt.else_body = else_body;
    return n;
  }

  if (match("for")) {
    skip();
    const char *first = expect_name(tokens, i, tk_num);

    if (match("=")) {
      skip();
      node_t *start = p_exp(tokens, i, tk_num);
      expect(tokens, i, tk_num, ",");
      node_t *limit = p_exp(tokens, i, tk_num);
      node_t *step = NULL;
      if (match(",")) {
        skip();
        step = p_exp(tokens, i, tk_num);
      }
      expect(tokens, i, tk_num, "do");
      node_t *body = p_block(tokens, i, tk_num);
      expect(tokens, i, tk_num, "end");
      node_alloc(n);
      n->kind = SOL_NODE_FOR_NUM;
      n->u.for_num.name = first;
      n->u.for_num.start = start;
      n->u.for_num.limit = limit;
      n->u.for_num.step = step;
      n->u.for_num.body = body;
      return n;
    }

    list_t *names = sol_list_init();
    sol_list_push(names, (void *)first);
    while (match(",")) {
      skip();
      sol_list_push(names, (void *)expect_name(tokens, i, tk_num));
    }
    expect(tokens, i, tk_num, "in");
    node_t **iters;
    size iter_n;
    p_explist_into(tokens, i, tk_num, &iters, &iter_n);
    expect(tokens, i, tk_num, "do");
    node_t *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "end");
    node_alloc(n);
    n->kind = SOL_NODE_FOR_IN;
    n->u.for_in.names = (const char **)names->raw;
    n->u.for_in.name_n = names->num;
    n->u.for_in.iters = iters;
    n->u.for_in.iter_n = iter_n;
    n->u.for_in.body = body;
    return n;
  }

  if (match("function")) {
    skip();
    list_t *path = sol_list_init();
    sol_list_push(path, (void *)expect_name(tokens, i, tk_num));
    while (match(".")) {
      skip();
      sol_list_push(path, (void *)expect_name(tokens, i, tk_num));
    }
    const char *method = NULL;
    if (match(":")) {
      skip();
      method = expect_name(tokens, i, tk_num);
    }
    node_t *body = p_funcbody(tokens, i, tk_num);
    node_alloc(n);
    n->kind = SOL_NODE_FUNC;
    n->u.func.path = (const char **)path->raw;
    n->u.func.path_n = path->num;
    n->u.func.method = method;
    n->u.func.body = body;
    return n;
  }

  if (match("local") || match("global")) {
    int is_global = match("global");
    skip();

    if (match("function")) {
      skip();
      const char *name = expect_name(tokens, i, tk_num);
      node_t *body = p_funcbody(tokens, i, tk_num);
      node_alloc(n);
      if (is_global) {
        n->kind = SOL_NODE_FUNC;
      } else {
        n->kind = SOL_NODE_LOCAL_FUNC;
      }
      n->u.func.path = malloc(sizeof(char *));
      n->u.func.path[0] = name;
      n->u.func.path_n = 1;
      n->u.func.method = NULL;
      n->u.func.body = body;
      return n;
    }

    if (is_global && match("*")) {
      skip();
      node_alloc(n);
      n->kind = SOL_NODE_GLOBAL_WILDCARD;
      return n;
    }

    list_t *names = sol_list_init();
    list_t *attribs = sol_list_init();

    const char *first_attrib = p_attrib(tokens, i, tk_num);
    const char *first_name = expect_name(tokens, i, tk_num);
    const char *post_attrib = p_attrib(tokens, i, tk_num);
    const char *resolved = first_attrib ? first_attrib : post_attrib;

    sol_list_push(names, (void *)first_name);
    sol_list_push(attribs, (void *)resolved);

    while (match(",")) {
      skip();
      const char *pre = p_attrib(tokens, i, tk_num);
      const char *name = expect_name(tokens, i, tk_num);
      const char *post = p_attrib(tokens, i, tk_num);
      sol_list_push(names, (void *)name);
      sol_list_push(attribs, (void *)(pre ? pre : post));
    }

    node_t **values = NULL;
    size value_n = 0;
    if (match("=")) {
      skip();
      p_explist_into(tokens, i, tk_num, &values, &value_n);
    }

    node_alloc(n);
    n->kind = SOL_NODE_DECL;
    n->u.decl.names = (const char **)names->raw;
    n->u.decl.attribs = (const char **)attribs->raw;
    n->u.decl.n = names->num;
    n->u.decl.values = values;
    n->u.decl.value_n = value_n;
    n->u.decl.is_global = is_global;
    return n;
  }

  // varlist '=' explist  |  functioncall
  node_t *first = p_prefixexp(tokens, i, tk_num);

  if (match("=") || match(",")) {
    list_t *targets = sol_list_init();
    sol_list_push(targets, first);
    while (match(",")) {
      skip();
      sol_list_push(targets, p_prefixexp(tokens, i, tk_num));
    }
    expect(tokens, i, tk_num, "=");
    node_t **values;
    size value_n;
    p_explist_into(tokens, i, tk_num, &values, &value_n);
    node_alloc(n);
    n->kind = SOL_NODE_ASSIGN;
    n->u.assign.targets = (node_t **)targets->raw;
    n->u.assign.target_n = targets->num;
    n->u.assign.values = values;
    n->u.assign.value_n = value_n;
    return n;
  }

  if (first->kind == SOL_NODE_FUNC_CALL || first->kind == SOL_NODE_METHOD_CALL)
    return first;

  sol_fatal("unrecognized token '%s' at line %lu\n", cur_txt(),
            tokens[*i]->line);
  return NULL;
}

node_t **sol_parse(token_t **tokens, uint64 tk_num, uint64 *node_num) {
  list_t *nodes = sol_list_init();
  size i = 0;

  node_t *root = p_block(tokens, &i, (size)tk_num);
  sol_list_push(nodes, root);

  if (node_num)
    *node_num = nodes->num;
  return (node_t **)nodes->raw;
}
