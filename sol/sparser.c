#include "sparser.h"
#include "common/arena.h"
#include "common/error.h"
#include "common/types.h"
#include "common/vector.h"
#include "slexer.h"
#include "snode.h"
#include <stdlib.h>
#include <string.h>

parser_t *new_parser(lexer_t *lexer) {
  parser_t *parser = malloc(sizeof(parser_t));
  parser->arena = new_arena();
  parser->nodes = new_vector(sizeof(node_t));
  parser->ntoken = vector_size(lexer->tokens);
  parser->i = 0;

  parser->lexer = lexer;
  return parser;
}

static node_t *new_node(parser_t *parser) {
  return arena_alloc(parser->arena, sizeof(node_t));
}

static const char *dup_str(parser_t *parser, const char *s) {
  ulong len = strlen(s);
  char *out = arena_alloc(parser->arena, len + 1);
  memcpy(out, s, len + 1);
  return out;
}

static vector_t *nv_new_nodes(void) { return new_vector(sizeof(node_t *)); }
static vector_t *nv_new_strs(void) { return new_vector(sizeof(const char *)); }

static void nv_push_node(vector_t *v, node_t *n) { push_back(v, &n); }
static void nv_push_str(vector_t *v, const char *s) { push_back(v, &s); }

static node_t *nv_get_node(vector_t *v, ulong idx) {
  return *(node_t **)vector_get(v, idx);
}
static const char *nv_get_str(vector_t *v, ulong idx) {
  return *(const char **)vector_get(v, idx);
}

static node_t **nv_to_node_array(parser_t *parser, vector_t *v, ulong *out_n) {
  ulong n = vector_size(v);
  node_t **arr = arena_alloc(parser->arena, sizeof(node_t *) * (n ? n : 1));
  for (ulong j = 0; j < n; j++)
    arr[j] = nv_get_node(v, j);
  *out_n = n;
  free_vector(v);
  return arr;
}

static const char **nv_to_str_array(parser_t *parser, vector_t *v,
                                    ulong *out_n) {
  ulong n = vector_size(v);
  const char **arr =
      arena_alloc(parser->arena, sizeof(const char *) * (n ? n : 1));
  for (ulong j = 0; j < n; j++)
    arr[j] = nv_get_str(v, j);
  *out_n = n;
  free_vector(v);
  return arr;
}

#define tk(i) ((token_t *)vector_get(parser->lexer->tokens, i))
#define skip() parser->i++
#define node_alloc(N) node_t *N = new_node(parser)
#define cur_tk() tk(parser->i)
#define cur_txt() (cur_tk()->txt)
#define cur_type() (cur_tk()->type)
#define match(TXT) (parser->i < parser->ntoken && strcmp(cur_txt(), TXT) == 0)
#define match_type(T) (parser->i < parser->ntoken && cur_type() == (T))
#define at_end() (parser->i >= parser->ntoken || cur_type() == SOL_TK_EOF)

static node_t *p_exp(parser_t *parser);
static node_t *p_block(parser_t *parser);
static node_t *p_stmt(parser_t *parser);
static node_t *p_prefixexp(parser_t *parser);

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

static void expect(parser_t *parser, const char *txt) {
  if (at_end() || !match(txt))
    sol_fatal("expected '%s', got '%s' at line %lu\n", txt, cur_txt(),
              cur_tk()->line);
  skip();
}

static const char *expect_name(parser_t *parser) {
  if (at_end() || cur_type() != SOL_TK_IDENT)
    sol_fatal("expected name, got '%s' at line %lu\n", cur_txt(),
              cur_tk()->line);
  const char *name = dup_str(parser, cur_txt());
  skip();
  return name;
}

static const char *p_attrib(parser_t *parser) {
  if (!match("<"))
    return NULL;
  skip();
  const char *name = expect_name(parser);
  expect(parser, ">");
  return name;
}

static node_t **p_explist_into(parser_t *parser, ulong *out_n) {
  vector_t *list = nv_new_nodes();
  nv_push_node(list, p_exp(parser));
  while (match(",")) {
    skip();
    nv_push_node(list, p_exp(parser));
  }
  return nv_to_node_array(parser, list, out_n);
}

static const char **p_namelist_into(parser_t *parser, ulong *out_n) {
  vector_t *list = nv_new_strs();
  nv_push_str(list, expect_name(parser));
  while (match(",")) {
    skip();
    nv_push_str(list, expect_name(parser));
  }
  return nv_to_str_array(parser, list, out_n);
}

static node_t *p_funcbody(parser_t *parser) {
  expect(parser, "(");

  vector_t *params = nv_new_strs();
  int has_vararg = 0;
  const char *vararg_name = NULL;

  if (!match(")")) {
    if (match("...")) {
      has_vararg = 1;
      skip();
      if (match_type(SOL_TK_IDENT)) {
        vararg_name = dup_str(parser, cur_txt());
        skip();
      }
    } else {
      nv_push_str(params, expect_name(parser));
      while (match(",")) {
        skip();
        if (match("...")) {
          has_vararg = 1;
          skip();
          if (match_type(SOL_TK_IDENT)) {
            vararg_name = dup_str(parser, cur_txt());
            skip();
          }
          break;
        }
        nv_push_str(params, expect_name(parser));
      }
    }
  }

  expect(parser, ")");
  node_t *body = p_block(parser);
  expect(parser, "end");

  ulong param_n;
  const char **param_arr = nv_to_str_array(parser, params, &param_n);

  node_alloc(node);
  node->kind = SOL_NODE_FUNC_DEF;
  node->u.funcbody.params = param_arr;
  node->u.funcbody.param_n = param_n;
  node->u.funcbody.has_vararg = has_vararg;
  node->u.funcbody.vararg_name = vararg_name;
  node->u.funcbody.body = body;
  return node;
}

static node_t *p_table(parser_t *parser) {
  expect(parser, "{");
  vector_t *fields = nv_new_nodes();

  while (!match("}")) {
    node_alloc(field);

    if (match("[")) {
      skip();
      node_t *key = p_exp(parser);
      expect(parser, "]");
      expect(parser, "=");
      node_t *val = p_exp(parser);
      field->kind = SOL_NODE_TABLE_FIELD;
      field->u.table_field.key = key;
      field->u.table_field.val = val;
    } else if (cur_type() == SOL_TK_IDENT && !at_end() &&
               strcmp(tk(parser->i + 1)->txt, "=") == 0) {
      const char *name = dup_str(parser, cur_txt());
      skip();
      skip(); // '='
      node_t *val = p_exp(parser);
      field->kind = SOL_NODE_FIELD;
      field->u.field.name = name;
      field->u.field.target = val;
    } else {
      node_t *val = p_exp(parser);
      *field = *val;
    }

    nv_push_node(fields, field);

    if (match(",") || match(";"))
      skip();
    else
      break;
  }

  expect(parser, "}");

  ulong n;
  node_t **arr = nv_to_node_array(parser, fields, &n);

  node_alloc(node);
  node->kind = SOL_NODE_TABLE;
  node->u.table.fields = arr;
  node->u.table.n = n;
  return node;
}

static node_t **p_args(parser_t *parser, ulong *arg_n) {
  vector_t *args = nv_new_nodes();

  if (match("(")) {
    skip();
    if (!match(")")) {
      nv_push_node(args, p_exp(parser));
      while (match(",")) {
        skip();
        nv_push_node(args, p_exp(parser));
      }
    }
    expect(parser, ")");
  } else if (match("{")) {
    nv_push_node(args, p_table(parser));
  } else if (cur_type() == SOL_TK_STRING) {
    node_alloc(s);
    s->kind = SOL_NODE_STRING;
    s->u.str = dup_str(parser, cur_txt());
    skip();
    nv_push_node(args, s);
  } else {
    sol_fatal("expected function arguments, got '%s' at line %lu\n", cur_txt(),
              cur_tk()->line);
  }

  return nv_to_node_array(parser, args, arg_n);
}

static node_t *p_primaryexp(parser_t *parser) {
  if (cur_type() == SOL_TK_IDENT) {
    node_alloc(node);
    node->kind = SOL_NODE_IDENT;
    node->u.str = dup_str(parser, cur_txt());
    skip();
    return node;
  }

  if (match("(")) {
    skip();
    node_t *inner = p_exp(parser);
    expect(parser, ")");
    return inner;
  }

  sol_fatal("unexpected symbol '%s' at line %lu\n", cur_txt(), cur_tk()->line);
  return NULL;
}

static node_t *p_prefixexp(parser_t *parser) {
  node_t *node = p_primaryexp(parser);

  while (!at_end()) {
    if (match(".")) {
      skip();
      const char *name = expect_name(parser);
      node_alloc(field);
      field->kind = SOL_NODE_FIELD;
      field->u.field.target = node;
      field->u.field.name = name;
      node = field;
    } else if (match("[")) {
      skip();
      node_t *key = p_exp(parser);
      expect(parser, "]");
      node_alloc(idx);
      idx->kind = SOL_NODE_INDEX;
      idx->u.index.target = node;
      idx->u.index.key = key;
      node = idx;
    } else if (match(":")) {
      skip();
      const char *method = expect_name(parser);
      ulong arg_n;
      node_t **args = p_args(parser, &arg_n);
      node_alloc(call);
      call->kind = SOL_NODE_METHOD_CALL;
      call->u.method_call.target = node;
      call->u.method_call.method = method;
      call->u.method_call.args = args;
      call->u.method_call.arg_n = arg_n;
      node = call;
    } else if (match("(") || match("{") || cur_type() == SOL_TK_STRING) {
      ulong arg_n;
      node_t **args = p_args(parser, &arg_n);
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

static node_t *p_simple_exp(parser_t *parser) {
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
    n->u.str = dup_str(parser, cur_txt());
    skip();
    return n;
  }
  if (match("function")) {
    skip();
    return p_funcbody(parser);
  }
  if (match("{")) {
    return p_table(parser);
  }
  if (is_unop(cur_txt())) {
    token_t *op = cur_tk();
    skip();
    node_t *operand = p_simple_exp(parser);
    node_alloc(n);
    n->kind = SOL_NODE_UNOP;
    n->u.unop.op = op;
    n->u.unop.operand = operand;
    return n;
  }

  return p_prefixexp(parser);
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

static node_t *p_exp_prec(parser_t *parser, int min_prec) {
  node_t *left = p_simple_exp(parser);

  while (!at_end() && is_binop(cur_txt())) {
    const char *op_txt = cur_txt();
    int prec = binop_precedence(op_txt);
    if (prec < min_prec)
      break;

    token_t *op = cur_tk();
    skip();

    int next_prec = is_right_assoc(op_txt) ? prec : prec + 1;
    node_t *right = p_exp_prec(parser, next_prec);

    node_alloc(bin);
    bin->kind = SOL_NODE_BINOP;
    bin->u.binop.left = left;
    bin->u.binop.right = right;
    bin->u.binop.op = op;
    left = bin;
  }

  return left;
}

static node_t *p_exp(parser_t *parser) { return p_exp_prec(parser, 1); }

static node_t *p_block(parser_t *parser) {
  vector_t *list = nv_new_nodes();
  node_t *retstat = NULL;

  while (!at_end() && !match("end") && !match("else") && !match("elseif") &&
         !match("until")) {
    if (match("return")) {
      skip();

      vector_t *rets = nv_new_nodes();
      if (!at_end() && !match(";") && !match("end") && !match("else") &&
          !match("elseif") && !match("until")) {
        nv_push_node(rets, p_exp(parser));
        while (match(",")) {
          skip();
          nv_push_node(rets, p_exp(parser));
        }
      }

      ulong ret_n;
      node_t **ret_arr = nv_to_node_array(parser, rets, &ret_n);

      node_alloc(rs);
      rs->kind = SOL_NODE_RETURN;
      rs->u.ret.explist = ret_arr;
      rs->u.ret.n = ret_n;
      retstat = rs;

      if (match(";"))
        skip();
      break;
    }

    node_t *s = p_stmt(parser);
    if (s)
      nv_push_node(list, s);
  }

  ulong n;
  node_t **arr = nv_to_node_array(parser, list, &n);

  node_alloc(block);
  block->kind = SOL_NODE_BLOCK;
  block->u.block.stmts = arr;
  block->u.block.n = n;
  block->u.block.retstat = retstat;
  return block;
}

static node_t *p_stmt(parser_t *parser) {
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
    n->u.str = expect_name(parser);
    return n;
  }

  if (match("::")) {
    skip();
    const char *name = expect_name(parser);
    expect(parser, "::");
    node_alloc(n);
    n->kind = SOL_NODE_LABEL;
    n->u.str = name;
    return n;
  }

  if (match("do")) {
    skip();
    node_t *body = p_block(parser);
    expect(parser, "end");
    node_alloc(n);
    n->kind = SOL_NODE_DO;
    n->u.block = body->u.block;
    return n;
  }

  if (match("while")) {
    skip();
    node_t *cond = p_exp(parser);
    expect(parser, "do");
    node_t *body = p_block(parser);
    expect(parser, "end");
    node_alloc(n);
    n->kind = SOL_NODE_WHILE;
    n->u.while_loop.cond = cond;
    n->u.while_loop.body = body;
    return n;
  }

  if (match("repeat")) {
    skip();
    node_t *body = p_block(parser);
    expect(parser, "until");
    node_t *cond = p_exp(parser);
    node_alloc(n);
    n->kind = SOL_NODE_REPEAT;
    n->u.repeat_loop.body = body;
    n->u.repeat_loop.cond = cond;
    return n;
  }

  if (match("if")) {
    skip();
    vector_t *conds = nv_new_nodes();
    vector_t *bodies = nv_new_nodes();

    nv_push_node(conds, p_exp(parser));
    expect(parser, "then");
    nv_push_node(bodies, p_block(parser));

    while (match("elseif")) {
      skip();
      nv_push_node(conds, p_exp(parser));
      expect(parser, "then");
      nv_push_node(bodies, p_block(parser));
    }

    node_t *else_body = NULL;
    if (match("else")) {
      skip();
      else_body = p_block(parser);
    }

    expect(parser, "end");

    ulong cond_n, body_n;
    node_t **cond_arr = nv_to_node_array(parser, conds, &cond_n);
    node_t **body_arr = nv_to_node_array(parser, bodies, &body_n);

    node_alloc(n);
    n->kind = SOL_NODE_IF;
    n->u.if_stmt.conds = cond_arr;
    n->u.if_stmt.bodies = body_arr;
    n->u.if_stmt.n = cond_n;
    n->u.if_stmt.else_body = else_body;
    return n;
  }

  if (match("for")) {
    skip();
    const char *first = expect_name(parser);

    if (match("=")) {
      skip();
      node_t *start = p_exp(parser);
      expect(parser, ",");
      node_t *limit = p_exp(parser);
      node_t *step = NULL;
      if (match(",")) {
        skip();
        step = p_exp(parser);
      }
      expect(parser, "do");
      node_t *body = p_block(parser);
      expect(parser, "end");
      node_alloc(n);
      n->kind = SOL_NODE_FOR_NUM;
      n->u.for_num.name = first;
      n->u.for_num.start = start;
      n->u.for_num.limit = limit;
      n->u.for_num.step = step;
      n->u.for_num.body = body;
      return n;
    }

    vector_t *names = nv_new_strs();
    nv_push_str(names, first);
    while (match(",")) {
      skip();
      nv_push_str(names, expect_name(parser));
    }
    expect(parser, "in");

    ulong iter_n;
    node_t **iters = p_explist_into(parser, &iter_n);

    expect(parser, "do");
    node_t *body = p_block(parser);
    expect(parser, "end");

    ulong name_n;
    const char **name_arr = nv_to_str_array(parser, names, &name_n);

    node_alloc(n);
    n->kind = SOL_NODE_FOR_IN;
    n->u.for_in.names = name_arr;
    n->u.for_in.name_n = name_n;
    n->u.for_in.iters = iters;
    n->u.for_in.iter_n = iter_n;
    n->u.for_in.body = body;
    return n;
  }

  if (match("function")) {
    skip();
    vector_t *path = nv_new_strs();
    nv_push_str(path, expect_name(parser));
    while (match(".")) {
      skip();
      nv_push_str(path, expect_name(parser));
    }
    const char *method = NULL;
    if (match(":")) {
      skip();
      method = expect_name(parser);
    }
    node_t *body = p_funcbody(parser);

    ulong path_n;
    const char **path_arr = nv_to_str_array(parser, path, &path_n);

    node_alloc(n);
    n->kind = SOL_NODE_FUNC;
    n->u.func.path = path_arr;
    n->u.func.path_n = path_n;
    n->u.func.method = method;
    n->u.func.body = body;
    return n;
  }

  if (match("local")) {
    skip();

    if (match("function")) {
      skip();
      const char *name = expect_name(parser);
      node_t *body = p_funcbody(parser);

      const char **path = arena_alloc(parser->arena, sizeof(const char *));
      path[0] = name;

      node_alloc(n);
      n->kind = SOL_NODE_LOCAL_FUNC;
      n->u.func.path = path;
      n->u.func.path_n = 1;
      n->u.func.method = NULL;
      n->u.func.body = body;
      return n;
    }

    sol_fatal("expected 'function' after 'local' at line %lu\n",
              cur_tk()->line);
  }

  if (match("var")) {
    skip();

    vector_t *names = nv_new_strs();
    vector_t *attribs = nv_new_strs();

    const char *first_attrib = p_attrib(parser);
    const char *first_name = expect_name(parser);
    const char *post_attrib = p_attrib(parser);
    const char *resolved = first_attrib ? first_attrib : post_attrib;

    nv_push_str(names, first_name);
    nv_push_str(attribs, resolved);

    while (match(",")) {
      skip();
      const char *pre = p_attrib(parser);
      const char *name = expect_name(parser);
      const char *post = p_attrib(parser);
      nv_push_str(names, name);
      nv_push_str(attribs, pre ? pre : post);
    }

    node_t **values = NULL;
    ulong value_n = 0;
    if (match("=")) {
      skip();
      values = p_explist_into(parser, &value_n);
    }

    ulong name_n, attrib_n;
    const char **name_arr = nv_to_str_array(parser, names, &name_n);
    const char **attrib_arr = nv_to_str_array(parser, attribs, &attrib_n);

    if (name_n > value_n) {
      sol_fatal("less values than names in declaration\n");
    } else if (name_n < value_n) {
      sol_fatal("less names than values in declaration\n");
    }

    node_alloc(n);
    n->kind = SOL_NODE_DECL;
    n->u.decl.names = name_arr;
    n->u.decl.attribs = attrib_arr;
    n->u.decl.n = name_n;
    n->u.decl.values = values;
    n->u.decl.value_n = value_n;
    return n;
  }

  node_t *first = p_prefixexp(parser);

  if (match("=") || match(",")) {
    vector_t *targets = nv_new_nodes();
    nv_push_node(targets, first);
    while (match(",")) {
      skip();
      nv_push_node(targets, p_prefixexp(parser));
    }
    expect(parser, "=");

    ulong value_n;
    node_t **values = p_explist_into(parser, &value_n);

    ulong target_n;
    node_t **target_arr = nv_to_node_array(parser, targets, &target_n);

    node_alloc(n);
    n->kind = SOL_NODE_ASSIGN;
    n->u.assign.targets = target_arr;
    n->u.assign.target_n = target_n;
    n->u.assign.values = values;
    n->u.assign.value_n = value_n;
    return n;
  }

  if (first->kind == SOL_NODE_FUNC_CALL || first->kind == SOL_NODE_METHOD_CALL)
    return first;

  sol_fatal("unrecognized token '%s' at line %lu\n", cur_txt(), cur_tk()->line);
  return NULL;
}

void parse(parser_t *parser) {
  parser->ntoken = vector_size(parser->lexer->tokens);
  parser->i = 0;
  parser->nodes = new_vector(sizeof(node_t *));

  node_t *root = p_block(parser);
  push_back(parser->nodes, &root);
}
