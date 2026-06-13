#include "pluja/lexer/lexer.h"
#include "pluja/types.h"
#include <stdio.h>

int main(void) {
  uint64 tk_num;
  token_t **tokens = plj_lex("_hello123_hi x", &tk_num);

  for (idx_t i = 0; i < tk_num; i++) {
    printf("txt: %s\n", tokens[i]->txt);
  }

  plj_cleanup_lex_res(tokens, tk_num);
  return 0;
}
