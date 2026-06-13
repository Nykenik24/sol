#include "pluja/lexer/lexer.h"
#include "pluja/lexer/token.h"
#include "pluja/types.h"
#include <stdio.h>

int main(void) {
  uint64 tk_num;
  token_t **tokens = plj_lex("_hello123_hi x '123 \"hello\"'", &tk_num);

  for (idx_t i = 0; i < tk_num; i++) {
    token_t *tk = tokens[i];
    printf("txt: '%s', type: %s\n", tk->txt, plj_ttype_to_str(tk->type));
  }

  plj_cleanup_lex_res(tokens, tk_num);
  return 0;
}
