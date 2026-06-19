#ifndef _INCLUDE_NODE_H_
#define _INCLUDE_NODE_H_

#include "common/types.h"
#include "slexer.h"

typedef enum node_kind_t {
  SOL_NODE_IDENT,
  SOL_NODE_STRING,
  SOL_NODE_DIGIT,
  SOL_NODE_HEX_DIGIT,
  SOL_NODE_NIL,
  SOL_NODE_TRUE,
  SOL_NODE_FALSE,
  SOL_NODE_VARARG,
  SOL_NODE_BINOP,
  SOL_NODE_UNOP,
  SOL_NODE_BREAK,
  SOL_NODE_GOTO,
  SOL_NODE_LABEL,
  SOL_NODE_BLOCK,
  SOL_NODE_DO,
  SOL_NODE_WHILE,
  SOL_NODE_REPEAT,
  SOL_NODE_IF,
  SOL_NODE_FOR_NUM,
  SOL_NODE_FOR_IN,
  SOL_NODE_RETURN,
  SOL_NODE_ASSIGN,
  SOL_NODE_LOCAL,
  SOL_NODE_GLOBAL,
  SOL_NODE_FUNC,
  SOL_NODE_LOCAL_FUNC,
  SOL_NODE_FUNC_DEF,
  SOL_NODE_FUNC_CALL,
  SOL_NODE_METHOD_CALL,
  SOL_NODE_INDEX,
  SOL_NODE_FIELD,
  SOL_NODE_TABLE,
  SOL_NODE_ATTRIB,
  SOL_NODE_DECL,
  SOL_NODE_TABLE_FIELD,
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
      ulong n;
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
      ulong n;
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
      ulong name_n;
      struct node_t **iters;
      ulong iter_n;
      struct node_t *body;
    } for_in;

    struct {
      struct node_t **explist;
      ulong n;
    } ret;

    struct {
      struct node_t **targets;
      ulong target_n;
      struct node_t **values;
      ulong value_n;
    } assign;

    struct {
      const char **names;
      const char **attribs;
      ulong n;
      struct node_t **values;
      ulong value_n;
    } decl;

    struct {
      const char **path;
      ulong path_n;
      const char *method;
      struct node_t *body;
    } func;

    struct {
      const char **params;
      ulong param_n;
      int has_vararg;
      const char *vararg_name;
      struct node_t *body;
    } funcbody;

    struct {
      struct node_t *target;
      struct node_t **args;
      ulong arg_n;
    } call;

    struct {
      struct node_t *target;
      const char *method;
      struct node_t **args;
      ulong arg_n;
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
      ulong n;
    } table;

    struct {
      const char *name;
      const char *attrib;
    } attrib;
  } u;
} node_t;

#endif // !_INCLUDE_NODE_H_
