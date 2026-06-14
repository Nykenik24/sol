#include "pluja/error.h"
#include "pluja/lexer/lexer.h"
#include "pluja/lexer/token.h"
#include "pluja/parser/node.h"
#include "pluja/parser/parser.h"
#include "pluja/types.h"
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
    plj_fatal("expected filename\n");

  char *source = read_file(argv[1]);

  uint64 tk_num;
  token_t **tokens = plj_lex(source, &tk_num);

  // for (size i = 0; i < tk_num; i++) {
  //   token_t *tk = tokens[i];
  //   printf("txt: %s, type: %s\n", tk->txt, plj_ttype_to_str(tk->type));
  // }

  uint64 node_num;
  node_t **nodes = plj_parse(tokens, tk_num, &node_num);
  plj_print_ast(nodes, node_num);

  plj_cleanup_lex_res(tokens, tk_num);
  plj_cleanup_parse_res(nodes, node_num);
  return 0;
}
