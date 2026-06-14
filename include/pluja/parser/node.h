#ifndef PLUJA_INCLUDE__PARSER_NODE_H
#define PLUJA_INCLUDE__PARSER_NODE_H

#include "pluja/lexer/token.h"
#include "pluja/types.h"

typedef enum {
  PLJ_NODE_IDENT,
  PLJ_NODE_STRING,
  PLJ_NODE_DIGIT,
  PLJ_NODE_HEX_DIGIT,
  PLJ_NODE_NIL,
  PLJ_NODE_TRUE,
  PLJ_NODE_FALSE,
  PLJ_NODE_VARARG,
  PLJ_NODE_BINOP,
  PLJ_NODE_UNOP,
  PLJ_NODE_BREAK,
  PLJ_NODE_GOTO,
  PLJ_NODE_LABEL,
  PLJ_NODE_BLOCK,
  PLJ_NODE_DO,
  PLJ_NODE_WHILE,
  PLJ_NODE_REPEAT,
  PLJ_NODE_IF,
  PLJ_NODE_FOR_NUM,
  PLJ_NODE_FOR_IN,
  PLJ_NODE_RETURN,
  PLJ_NODE_ASSIGN,
  PLJ_NODE_LOCAL,
  PLJ_NODE_GLOBAL,
  PLJ_NODE_GLOBAL_WILDCARD,
  PLJ_NODE_FUNC,
  PLJ_NODE_LOCAL_FUNC,
  PLJ_NODE_FUNC_DEF,
  PLJ_NODE_FUNC_CALL,
  PLJ_NODE_METHOD_CALL,
  PLJ_NODE_INDEX,
  PLJ_NODE_FIELD,
  PLJ_NODE_TABLE,
  PLJ_NODE_ATTRIB,
  PLJ_NODE_DECL,
  PLJ_NODE_TABLE_FIELD,
} node_kind_t;

typedef struct node_t {
  node_kind_t kind;
  union {
    double num;
    const char *str;

    struct {
      struct node_t *left, *right;
      token_t *op;
    } binop;

    struct {
      struct node_t *operand;
      token_t *op;
    } unop;

    struct {
      struct node_t **stmts;
      size n;
      struct node_t *retstat;
    } block;

    struct {
      struct node_t *cond;
      struct node_t *body;
    } while_loop;

    struct {
      struct node_t *body;
      struct node_t *cond;
    } repeat_loop;

    struct {
      struct node_t **conds;
      struct node_t **bodies;
      size n;
      struct node_t *else_body;
    } if_stmt;

    struct {
      const char *name;
      struct node_t *start;
      struct node_t *limit;
      struct node_t *step;
      struct node_t *body;
    } for_num;

    struct {
      const char **names;
      size name_n;
      struct node_t **iters;
      size iter_n;
      struct node_t *body;
    } for_in;

    struct {
      struct node_t **explist;
      size n;
    } ret;

    struct {
      struct node_t **targets;
      size target_n;
      struct node_t **values;
      size value_n;
    } assign;

    struct {
      const char **names;
      const char **attribs;
      size n;
      struct node_t **values;
      size value_n;
      int is_global;
    } decl;

    struct {
      const char **path;
      size path_n;
      const char *method;
      struct node_t *body;
    } func;

    struct {
      struct node_t **params;
      size param_n;
      int has_vararg;
      const char *vararg_name;
      struct node_t *body;
    } funcbody;

    struct {
      struct node_t *target;
      struct node_t **args;
      size arg_n;
    } call;

    struct {
      struct node_t *target;
      const char *method;
      struct node_t **args;
      size arg_n;
    } method_call;

    struct {
      struct node_t *target;
      struct node_t *key;
    } index;

    struct {
      struct node_t *target;
      const char *name;
    } field;

    struct {
      struct node_t *key;
      struct node_t *val;
    } table_field;

    struct {
      struct node_t **fields;
      size n;
    } table;

    struct {
      const char *name;
      const char *attrib;
    } attrib;

  } u;
} node_t;

// Destroy a parser node.
void plj_node_destroy(node_t *node);

#endif // !PLUJA_INCLUDE__PARSER_NODE_H
