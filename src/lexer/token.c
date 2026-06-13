#include "pluja/lexer/token.h"
#include "pluja/types.h"
#include <stdlib.h>
#include <string.h>

token_t *plj_token_create(char *txt, uint64 len, token_type_t type) {
  token_t *tk = malloc(sizeof(token_t));
  tk->txt = strdup(txt);
  tk->len = len;
  tk->type = type;
  return tk;
}

token_t *plj_token_eof() {
  token_t *tk = malloc(sizeof(token_t));
  char buf[1] = {'\0'};
  tk->txt = strdup(buf);
  tk->type = PLJ_TK_EOF;
  return tk;
}

void plj_token_destroy(token_t *tk) {
  if (!tk)
    return;

  if (tk->txt)
    free(tk->txt);

  free(tk);
}

const char *tstrtable[] = {
    [PLJ_TK_EOF] = "EOF",       [PLJ_TK_IDENT] = "IDENT",
    [PLJ_TK_STRING] = "STRING", [PLJ_TK_DIGIT] = "DIGIT",
    [PLJ_TK_SYMBOL] = "SYMBOL",
};
uint64 tstrtable_len = (sizeof tstrtable / sizeof tstrtable[0]);

char *plj_ttype_to_str(token_type_t type) {
  if (type >= tstrtable_len)
    return "UNKNOWN";
  return (char *)tstrtable[type];
}
