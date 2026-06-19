#include <stdlib.h>

#include "common/arena.h"
#include "common/vector.h"
#include "slexer.h"
#include "sparser.h"

int main(void) {
  lexer_t *lexer = new_lexer();
  lex(lexer, "var x, y = 5, 4\nprint(x, y)");
  parser_t *parser = new_parser(lexer);
  parse(parser);
  for (ulong i = 0; i < vector_size(parser->nodes); i++) {
    print_ast(*(node_t **)vector_get(parser->nodes, i));
  }

  free_arena(lexer->arena);
  free_vector(lexer->tokens);
  free_arena(parser->arena);
  free_vector(parser->nodes);
  return EXIT_SUCCESS;
}
