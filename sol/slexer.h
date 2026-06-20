#ifndef _INCLUDE_LEXER_H_
#define _INCLUDE_LEXER_H_

#include "common/arena.h"
#include "common/types.h"
#include "common/vector.h"

typedef enum {
  SOL_TK_EOF,
  SOL_TK_IDENT,
  SOL_TK_STRING,
  SOL_TK_INT,
  SOL_TK_FLOAT,
  SOL_TK_HEX_DIGIT,

  SOL_TK_SYM_VARARG,
  SOL_TK_SYM_CONCAT,
  SOL_TK_SYM_LABEL,
  SOL_TK_SYM_EQUAL,
  SOL_TK_SYM_LEFTSHIFT,
  SOL_TK_SYM_RIGHTSHIFT,
  SOL_TK_SYM_NEQUAL,
  SOL_TK_SYM_LEQUAL,
  SOL_TK_SYM_GEQUAL,
  SOL_TK_SYM_ADD,
  SOL_TK_SYM_SUB,
  SOL_TK_SYM_MUL,
  SOL_TK_SYM_DIV,
  SOL_TK_SYM_MOD,
  SOL_TK_SYM_BITXOR,
  SOL_TK_SYM_BITAND,
  SOL_TK_SYM_BITNOT,
  SOL_TK_SYM_BITOR,
  SOL_TK_SYM_LEN,
  SOL_TK_SYM_LESS,
  SOL_TK_SYM_MORE,
  SOL_TK_SYM_LPAREN,
  SOL_TK_SYM_RPAREN,
  SOL_TK_SYM_LBRACKET,
  SOL_TK_SYM_RBRACKET,
  SOL_TK_SYM_LBRACE,
  SOL_TK_SYM_RBRACE,
  SOL_TK_SYM_ASSIGN,
  SOL_TK_SYM_SEMICOLON,
  SOL_TK_SYM_COLON,
  SOL_TK_SYM_COMMA,
  SOL_TK_SYM_PERIOD,

  SOL_TK_KW_FUNCTION,
  SOL_TK_KW_EXPORT,
  SOL_TK_KW_REPEAT,
  SOL_TK_KW_ELSEIF,
  SOL_TK_KW_RETURN,
  SOL_TK_KW_BREAK,
  SOL_TK_KW_FALSE,
  SOL_TK_KW_LOCAL,
  SOL_TK_KW_UNTIL,
  SOL_TK_KW_WHILE,
  SOL_TK_KW_ELSE,
  SOL_TK_KW_GOTO,
  SOL_TK_KW_THEN,
  SOL_TK_KW_TRUE,
  SOL_TK_KW_EACH,
  SOL_TK_KW_VAR,
  SOL_TK_KW_AND,
  SOL_TK_KW_END,
  SOL_TK_KW_NIL,
  SOL_TK_KW_NOT,
  SOL_TK_KW_DO,
  SOL_TK_KW_IF,
  SOL_TK_KW_IN,
  SOL_TK_KW_OR,
} token_type_t;

typedef struct {
  ulong line;
  ulong col;
  token_type_t type;
  const char *txt;
} token_t;

typedef struct {
  vector_t *tokens;
  arena_t *arena;
} lexer_t;

lexer_t *new_lexer();
void lex(lexer_t *lexer, const char *input);
void free_lexer(lexer_t *lexer);

#endif // !_INCLUDE_LEXER_H_
