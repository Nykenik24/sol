#include "sol/lexer/token.h"
#include "sol/types.h"
#include <stdlib.h>
#include <string.h>

Token *sol_token_create(char *txt, uint64 len, TokenType type) {
  Token *tk = malloc(sizeof(Token));
  tk->txt = strdup(txt);
  tk->len = len;
  tk->type = type;
  return tk;
}

Token *sol_token_eof() {
  Token *tk = malloc(sizeof(Token));
  char buf[1] = {'\0'};
  tk->txt = strdup(buf);
  tk->type = SOL_TK_EOF;
  return tk;
}

void sol_token_destroy(Token *tk) {
  if (!tk)
    return;

  if (tk->txt)
    free(tk->txt);

  free(tk);
}

const char *tstrtable[] = {
    [SOL_TK_EOF] = "EOF",       [SOL_TK_IDENT] = "IDENT",
    [SOL_TK_STRING] = "STRING", [SOL_TK_DIGIT] = "DIGIT",
    [SOL_TK_SYMBOL] = "SYMBOL", [SOL_TK_RESERVED] = "RESERVED",
};
uint64 tstrtable_len = (sizeof tstrtable / sizeof tstrtable[0]);

char *sol_ttype_to_str(TokenType type) {
  if (type >= tstrtable_len)
    return "UNKNOWN";
  return (char *)tstrtable[type];
}
