#ifndef _INCLUDE_NODE_H_
#define _INCLUDE_NODE_H_

#include "common/types.h"
#include "slexer.h"

typedef enum node_kind_t {
  SOL_NODE_IDENT,       // expr
  SOL_NODE_STRING,      // expr
  SOL_NODE_INT,         // expr
  SOL_NODE_FLOAT,       // expr
  SOL_NODE_HEX_DIGIT,   // expr
  SOL_NODE_NIL,         // expr
  SOL_NODE_TRUE,        // expr
  SOL_NODE_FALSE,       // expr
  SOL_NODE_VARARG,      // expr
  SOL_NODE_BINOP,       // expr
  SOL_NODE_UNOP,        // expr
  SOL_NODE_BREAK,       // stmt
  SOL_NODE_GOTO,        // stmt
  SOL_NODE_LABEL,       // stmt
  SOL_NODE_BLOCK,       // block
  SOL_NODE_DO,          // block
  SOL_NODE_WHILE,       // stmt
  SOL_NODE_REPEAT,      // stmt
  SOL_NODE_IF,          // stmt
  SOL_NODE_FOR_NUM,     // stmt
  SOL_NODE_FOR_IN,      // stmt
  SOL_NODE_RETURN,      // stmt
  SOL_NODE_ASSIGN,      // expr
  SOL_NODE_LOCAL,       // stmt
  SOL_NODE_FUNC,        // stmt
  SOL_NODE_LOCAL_FUNC,  // expr
  SOL_NODE_FUNC_DEF,    // stmt
  SOL_NODE_FUNC_CALL,   // expr
  SOL_NODE_METHOD_CALL, // expr
  SOL_NODE_INDEX,       // expr
  SOL_NODE_FIELD,       // expr
  SOL_NODE_TABLE,       // expr
  SOL_NODE_ATTRIB,      // only appears inside decl
  SOL_NODE_DECL,        // stmt
  SOL_NODE_TABLE_FIELD, // only appears inside table
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
      bool has_else;
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
