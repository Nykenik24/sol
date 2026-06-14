#include "sol/error.h"
#include "sol/lexer/lexer.h"
#include "sol/lexer/token.h"
#include "sol/parser/node.h"
#include "sol/parser/parser.h"
#include "sol/types.h"
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>

char *read_file(const char *path) {
  FILE *f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  rewind(f);
  char *buf = malloc(len + 1);
  fread(buf, 1, len, f);
  buf[len] = '\0';
  fclose(f);
  return buf;
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    sol_fatal("expected filename\n");

  char *source = read_file(argv[1]);

  uint64 tk_num;
  Token **tokens = sol_lex(source, &tk_num);

  // for (size i = 0; i < tk_num; i++) {
  //   Token *tk = tokens[i];
  //   printf("txt: %s, type: %s\n", tk->txt, sol_ttype_to_str(tk->type));
  // }

  uint64 node_num;
  Node **nodes = sol_parse(tokens, tk_num, &node_num);
  sol_print_ast(nodes, node_num);

  sol_cleanup_lex_res(tokens, tk_num);
  sol_cleanup_parse_res(nodes, node_num);
  return 0;
}
