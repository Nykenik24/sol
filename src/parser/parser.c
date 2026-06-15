#include "sol/parser/parser.h"
#include "sol/error.h"
#include "sol/lexer/token.h"
#include "sol/mem.h"
#include "sol/parser/node.h"
#include "sol/types.h"
#include <stdlib.h>
#include <string.h>

void sol_cleanup_parse_res(Node **nodes, uint64 node_num) {
  for (size i = 0; i < node_num; i++) {
    sol_node_destroy(nodes[i]);
  }
}

#define skip() (*i)++
#define node_alloc(N) Node *N = malloc(sizeof(Node))
#define match(TXT) (*i < tk_num && strcmp(tokens[*i]->txt, TXT) == 0)
#define match_type(T) (*i < tk_num && tokens[*i]->type == (T))
#define cur_txt() (tokens[*i]->txt)
#define cur_type() (tokens[*i]->type)
#define at_end() (*i >= tk_num || tokens[*i]->type == SOL_TK_EOF)

static Node *p_exp(Token **tokens, size *i, size tk_num);
static Node *p_block(Token **tokens, size *i, size tk_num);
static Node *p_stmt(Token **tokens, size *i, size tk_num);
static Node *p_prefixexp(Token **tokens, size *i, size tk_num);
static Node *p_functioncall(Token **tokens, size *i, size tk_num);

static const char *dup_str(const char *s) {
  size len = strlen(s);
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

static void expect(Token **tokens, size *i, size tk_num, const char *txt) {
  if (at_end() || !match(txt))
    sol_fatal("expected '%s '%s' at line %lu\n'", txt, cur_txt(),
              tokens[*i]->line);
  skip();
}

static const char *expect_name(Token **tokens, size *i, size tk_num) {
  if (at_end() || cur_type() != SOL_TK_IDENT)
    sol_fatal("expected name '%s' at line %lu\n", cur_txt(), tokens[*i]->line);
  const char *name = dup_str(cur_txt());
  skip();
  return name;
}

static const char *p_attrib(Token **tokens, size *i, size tk_num) {
  if (!match("<"))
    return NULL;
  skip();
  const char *name = expect_name(tokens, i, tk_num);
  expect(tokens, i, tk_num, ">");
  return name;
}

static Node *p_explist_into(Token **tokens, size *i, size tk_num, Node ***out,
                            size *out_n) {
  List *list = sol_list_create();
  sol_list_push(list, p_exp(tokens, i, tk_num));
  while (match(",")) {
    skip();
    sol_list_push(list, p_exp(tokens, i, tk_num));
  }
  *out = (Node **)list->raw;
  *out_n = list->num;
  return NULL;
}

static void p_namelist_into(Token **tokens, size *i, size tk_num,
                            const char ***out, size *out_n) {
  List *list = sol_list_create();
  sol_list_push(list, (void *)expect_name(tokens, i, tk_num));
  while (match(",") && tokens[*i + 1]->type == SOL_TK_IDENT) {
    skip();
    sol_list_push(list, (void *)expect_name(tokens, i, tk_num));
  }
  *out = (const char **)list->raw;
  *out_n = list->num;
}

static Node *p_funcbody(Token **tokens, size *i, size tk_num) {
  expect(tokens, i, tk_num, "(");

  List *params = sol_list_create();
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
  Node *body = p_block(tokens, i, tk_num);
  expect(tokens, i, tk_num, "end");

  node_alloc(node);
  node->kind = SOL_NODE_FUNC_DEF;
  node->u.funcbody.params = (Node **)params->raw;
  node->u.funcbody.param_n = params->num;
  node->u.funcbody.has_vararg = has_vararg;
  node->u.funcbody.vararg_name = vararg_name;
  node->u.funcbody.body = body;
  return node;
}

static Node *p_table(Token **tokens, size *i, size tk_num) {
  expect(tokens, i, tk_num, "{");
  List *fields = sol_list_create();

  while (!match("}")) {
    node_alloc(field);

    if (match("[")) {
      skip();
      Node *key = p_exp(tokens, i, tk_num);
      expect(tokens, i, tk_num, "]");
      expect(tokens, i, tk_num, "=");
      Node *val = p_exp(tokens, i, tk_num);
      field->kind = SOL_NODE_TABLE_FIELD;
      field->u.table_field.key = key;
      field->u.table_field.val = val;
    } else if (cur_type() == SOL_TK_IDENT && !at_end() &&
               strcmp(tokens[*i + 1]->txt, "=") == 0) {
      const char *name = dup_str(cur_txt());
      skip();
      skip(); // '='
      Node *val = p_exp(tokens, i, tk_num);
      field->kind = SOL_NODE_FIELD;
      field->u.field.name = name;
      field->u.field.target = val;
    } else {
      Node *val = p_exp(tokens, i, tk_num);
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
  node->u.table.fields = (Node **)fields->raw;
  node->u.table.n = fields->num;
  return node;
}

static Node **p_args(Token **tokens, size *i, size tk_num, size *arg_n) {
  List *args = sol_list_create();

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
  return (Node **)args->raw;
}

static Node *p_primaryexp(Token **tokens, size *i, size tk_num) {
  if (cur_type() == SOL_TK_IDENT) {
    node_alloc(node);
    node->kind = SOL_NODE_IDENT;
    node->u.str = dup_str(cur_txt());
    skip();
    return node;
  }

  if (match("(")) {
    skip();
    Node *inner = p_exp(tokens, i, tk_num);
    expect(tokens, i, tk_num, ")");
    return inner;
  }

  sol_fatal("unexpected symbol '%s' at line %lu\n", cur_txt(),
            tokens[*i]->line);
  return NULL;
}

static Node *p_prefixexp(Token **tokens, size *i, size tk_num) {
  Node *node = p_primaryexp(tokens, i, tk_num);

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
      Node *key = p_exp(tokens, i, tk_num);
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
      Node **args = p_args(tokens, i, tk_num, &arg_n);
      node_alloc(call);
      call->kind = SOL_NODE_METHOD_CALL;
      call->u.method_call.target = node;
      call->u.method_call.method = method;
      call->u.method_call.args = args;
      call->u.method_call.arg_n = arg_n;
      node = call;
    } else if (match("(") || match("{") || cur_type() == SOL_TK_STRING) {
      size arg_n;
      Node **args = p_args(tokens, i, tk_num, &arg_n);
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

static Node *p_simple_exp(Token **tokens, size *i, size tk_num) {
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
    Token *op = tokens[*i];
    skip();
    Node *operand = p_simple_exp(tokens, i, tk_num);
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

static Node *p_exp_prec(Token **tokens, size *i, size tk_num, int min_prec) {
  Node *left = p_simple_exp(tokens, i, tk_num);

  while (!at_end() && is_binop(cur_txt())) {
    const char *op_txt = cur_txt();
    int prec = binop_precedence(op_txt);
    if (prec < min_prec)
      break;

    Token *op = tokens[*i];
    skip();

    int next_prec = is_right_assoc(op_txt) ? prec : prec + 1;
    Node *right = p_exp_prec(tokens, i, tk_num, next_prec);

    node_alloc(bin);
    bin->kind = SOL_NODE_BINOP;
    bin->u.binop.left = left;
    bin->u.binop.right = right;
    bin->u.binop.op = op;
    left = bin;
  }

  return left;
}

static Node *p_exp(Token **tokens, size *i, size tk_num) {
  return p_exp_prec(tokens, i, tk_num, 1);
}

static Node *p_block(Token **tokens, size *i, size tk_num) {
  List *list = sol_list_create();
  Node *retstat = NULL;

  while (!at_end() && !match("end") && !match("else") && !match("elseif") &&
         !match("until")) {
    if (match("return")) {
      skip();
      retstat = malloc(sizeof(Node));
      retstat->kind = SOL_NODE_RETURN;
      List *rets = sol_list_create();
      if (!at_end() && !match(";") && !match("end") && !match("else") &&
          !match("elseif") && !match("until")) {
        sol_list_push(rets, p_exp(tokens, i, tk_num));
        while (match(",")) {
          skip();
          sol_list_push(rets, p_exp(tokens, i, tk_num));
        }
      }
      retstat->u.ret.explist = (Node **)rets->raw;
      retstat->u.ret.n = rets->num;
      if (match(";"))
        skip();
      break;
    }

    Node *s = p_stmt(tokens, i, tk_num);
    if (s)
      sol_list_push(list, s);
  }

  node_alloc(block);
  block->kind = SOL_NODE_BLOCK;
  block->u.block.stmts = (Node **)list->raw;
  block->u.block.n = list->num;
  block->u.block.retstat = retstat;
  return block;
}

static Node *p_stmt(Token **tokens, size *i, size tk_num) {
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
    Node *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "end");
    node_alloc(n);
    n->kind = SOL_NODE_DO;
    n->u.block = body->u.block;
    free(body);
    return n;
  }

  if (match("while")) {
    skip();
    Node *cond = p_exp(tokens, i, tk_num);
    expect(tokens, i, tk_num, "do");
    Node *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "end");
    node_alloc(n);
    n->kind = SOL_NODE_WHILE;
    n->u.while_loop.cond = cond;
    n->u.while_loop.body = body;
    return n;
  }

  if (match("repeat")) {
    skip();
    Node *body = p_block(tokens, i, tk_num);
    expect(tokens, i, tk_num, "until");
    Node *cond = p_exp(tokens, i, tk_num);
    node_alloc(n);
    n->kind = SOL_NODE_REPEAT;
    n->u.repeat_loop.body = body;
    n->u.repeat_loop.cond = cond;
    return n;
  }

  if (match("if")) {
    skip();
    List *conds = sol_list_create();
    List *bodies = sol_list_create();

    sol_list_push(conds, p_exp(tokens, i, tk_num));
    expect(tokens, i, tk_num, "then");
    sol_list_push(bodies, p_block(tokens, i, tk_num));

    while (match("elseif")) {
      skip();
      sol_list_push(conds, p_exp(tokens, i, tk_num));
      expect(tokens, i, tk_num, "then");
      sol_list_push(bodies, p_block(tokens, i, tk_num));
    }

    Node *else_body = NULL;
    if (match("else")) {
      skip();
      else_body = p_block(tokens, i, tk_num);
    }

    expect(tokens, i, tk_num, "end");

    node_alloc(n);
    n->kind = SOL_NODE_IF;
    n->u.if_stmt.conds = (Node **)conds->raw;
    n->u.if_stmt.bodies = (Node **)bodies->raw;
    n->u.if_stmt.n = conds->num;
    n->u.if_stmt.else_body = else_body;
    return n;
  }

  if (match("for")) {
    skip();
    const char *first = expect_name(tokens, i, tk_num);

    if (match("=")) {
      skip();
      Node *start = p_exp(tokens, i, tk_num);
      expect(tokens, i, tk_num, ",");
      Node *limit = p_exp(tokens, i, tk_num);
      Node *step = NULL;
      if (match(",")) {
        skip();
        step = p_exp(tokens, i, tk_num);
      }
      expect(tokens, i, tk_num, "do");
      Node *body = p_block(tokens, i, tk_num);
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

    List *names = sol_list_create();
    sol_list_push(names, (void *)first);
    while (match(",")) {
      skip();
      sol_list_push(names, (void *)expect_name(tokens, i, tk_num));
    }
    expect(tokens, i, tk_num, "in");
    Node **iters;
    size iter_n;
    p_explist_into(tokens, i, tk_num, &iters, &iter_n);
    expect(tokens, i, tk_num, "do");
    Node *body = p_block(tokens, i, tk_num);
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
    List *path = sol_list_create();
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
    Node *body = p_funcbody(tokens, i, tk_num);
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
      Node *body = p_funcbody(tokens, i, tk_num);
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

    List *names = sol_list_create();
    List *attribs = sol_list_create();

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

    Node **values = NULL;
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
  Node *first = p_prefixexp(tokens, i, tk_num);

  if (match("=") || match(",")) {
    List *targets = sol_list_create();
    sol_list_push(targets, first);
    while (match(",")) {
      skip();
      sol_list_push(targets, p_prefixexp(tokens, i, tk_num));
    }
    expect(tokens, i, tk_num, "=");
    Node **values;
    size value_n;
    p_explist_into(tokens, i, tk_num, &values, &value_n);
    node_alloc(n);
    n->kind = SOL_NODE_ASSIGN;
    n->u.assign.targets = (Node **)targets->raw;
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

Node **sol_parse(Token **tokens, uint64 tk_num, uint64 *node_num) {
  List *nodes = sol_list_create();
  size i = 0;

  Node *root = p_block(tokens, &i, (size)tk_num);
  sol_list_push(nodes, root);

  if (node_num)
    *node_num = nodes->num;
  return (Node **)nodes->raw;
}
