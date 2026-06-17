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
} NodeKind;

typedef struct Node {
  NodeKind kind;
  union {
    double num;
    const char *str;

    struct {
      struct Node *left, *right;
      Token *op;
    } binop;

    struct {
      struct Node *operand;
      Token *op;
    } unop;

    struct {
      struct Node **stmts;
      size n;
      struct Node *retstat;
    } block;

    struct {
      struct Node *cond;
      struct Node *body;
    } while_loop;

    struct {
      struct Node *body;
      struct Node *cond;
    } repeat_loop;

    struct {
      struct Node **conds;
      struct Node **bodies;
      size n;
      struct Node *else_body;
    } if_stmt;

    struct {
      const char *name;
      struct Node *start;
      struct Node *limit;
      struct Node *step;
      struct Node *body;
    } for_num;

    struct {
      const char **names;
      size name_n;
      struct Node **iters;
      size iter_n;
      struct Node *body;
    } for_in;

    struct {
      struct Node **explist;
      size n;
    } ret;

    struct {
      struct Node **targets;
      size target_n;
      struct Node **values;
      size value_n;
    } assign;

    struct {
      const char **names;
      const char **attribs;
      size n;
      struct Node **values;
      size value_n;
      int is_global;
    } decl;

    struct {
      const char **path;
      size path_n;
      const char *method;
      struct Node *body;
    } func;

    struct {
      struct Node **params;
      size param_n;
      int has_vararg;
      const char *vararg_name;
      struct Node *body;
    } funcbody;

    struct {
      struct Node *target;
      struct Node **args;
      size arg_n;
    } call;

    struct {
      struct Node *target;
      const char *method;
      struct Node **args;
      size arg_n;
    } method_call;

    struct {
      struct Node *target;
      struct Node *key;
    } index;

    struct {
      struct Node *target;
      const char *name;
    } field;

    struct {
      struct Node *key;
      struct Node *val;
    } table_field;

    struct {
      struct Node **fields;
      size n;
    } table;

    struct {
      const char *name;
      const char *attrib;
    } attrib;

  } u;
} Node;

// Destroy a parser node.
void sol_node_destroy(Node *node);

#endif // !SOL_INCLUDE__PARSER_NODE_H
