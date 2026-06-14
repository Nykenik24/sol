#ifndef SOL_INCLUDE__LEXER_TOKEN_H
#define SOL_INCLUDE__LEXER_TOKEN_H

#include "sol/types.h"
typedef enum {
  SOL_TK_EOF,
  SOL_TK_IDENT,
  SOL_TK_STRING,
  SOL_TK_DIGIT,
  SOL_TK_HEX_DIGIT,
  SOL_TK_SYMBOL,
  SOL_TK_RESERVED,
} token_type_t;

typedef struct {
  char *txt;
  uint64 len;
  uint64 line;
  token_type_t type;
} token_t;

// Creates a new lexer token.
token_t *sol_token_create(char *txt, uint64 len, token_type_t type);
// Creates a new EOF lexer token (type SOL_TK_EOF and txt is '\0').
token_t *sol_token_eof();
// Destroys a lexer token.
void sol_token_destroy(token_t *tk);

// Returns the token type as a string.
char *sol_ttype_to_str(token_type_t type);

#endif // !SOL_INCLUDE__LEXER_TOKEN_H
