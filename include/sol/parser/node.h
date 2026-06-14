#ifndef SOL_INCLUDE__PARSER_NODE_H
#define SOL_INCLUDE__PARSER_NODE_H

#include "sol/lexer/token.h"
#include "sol/types.h"

typedef enum {
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
  SOL_NODE_GLOBAL_WILDCARD,
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
void sol_node_destroy(node_t *node);

#endif // !SOL_INCLUDE__PARSER_NODE_H
