#ifndef PLUJA_INCLUDE__LEXER_TOKEN_H
#define PLUJA_INCLUDE__LEXER_TOKEN_H

#include "pluja/types.h"
typedef enum {
  PLJ_TK_EOF,
  PLJ_TK_IDENT,
} token_type_t;

typedef struct {
  char *txt;
  uint64 len;
  token_type_t type;
} token_t;

// Creates a new lexer token.
token_t *plj_token_create(char *txt, uint64 len, token_type_t type);
// Creates a new EOF lexer token (type PLJ_TK_EOF and txt is '\0').
token_t *plj_token_eof();
// Destroys a lexer token.
void plj_token_destroy(token_t *tk);

#endif // !PLUJA_INCLUDE__LEXER_TOKEN_H
