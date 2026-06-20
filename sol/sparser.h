#ifndef _INCLUDE_PARSER_H_
#define _INCLUDE_PARSER_H_

#include "slexer.h"
#include "snode.h"

typedef struct {
  node_t *ast;
  arena_t *arena;
  ulong ntoken;
  ulong i;

  lexer_t *lexer;
} parser_t;

parser_t *new_parser(lexer_t *lexer);
void parse(parser_t *parser);
void print_ast(node_t *root);
void free_parser(parser_t *parser);

#endif // !_INCLUDE_PARSER_H_
