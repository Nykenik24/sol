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
} TokenType;

typedef struct {
  char *txt;
  uint64 len;
  uint64 line;
  TokenType type;
} Token;

// Creates a new lexer token.
Token *sol_token_create(char *txt, uint64 len, TokenType type);
// Creates a new EOF lexer token (type SOL_TK_EOF and txt is '\0').
Token *sol_token_eof();
// Destroys a lexer token.
void sol_token_destroy(Token *tk);

// Returns the token type as a string.
char *sol_ttype_to_str(TokenType type);

#endif // !SOL_INCLUDE__LEXER_TOKEN_H
