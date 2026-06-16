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
    [SOL_TK_EOF] = "EOF",
    [SOL_TK_IDENT] = "IDENT",
    [SOL_TK_STRING] = "STRING",
    [SOL_TK_DIGIT] = "DIGIT",
    [SOL_TK_HEX_DIGIT] = "HEX_DIGIT",

    [SOL_TK_SYM_VARARG] = "VARARG",
    [SOL_TK_SYM_CONCAT] = "CONCAT",
    [SOL_TK_SYM_LABEL] = "LABEL",
    [SOL_TK_SYM_EQUAL] = "EQUAL",
    [SOL_TK_SYM_LEFTSHIFT] = "LEFTSHIFT",
    [SOL_TK_SYM_RIGHTSHIFT] = "RIGHTSHIFT",
    [SOL_TK_SYM_NEQUAL] = "NEQUAL",
    [SOL_TK_SYM_LEQUAL] = "LEQUAL",
    [SOL_TK_SYM_GEQUAL] = "GEQUAL",
    [SOL_TK_SYM_ADD] = "ADD",
    [SOL_TK_SYM_SUB] = "SUB",
    [SOL_TK_SYM_MUL] = "MUL",
    [SOL_TK_SYM_DIV] = "DIV",
    [SOL_TK_SYM_MOD] = "MOD",
    [SOL_TK_SYM_BITXOR] = "BITXOR",
    [SOL_TK_SYM_BITAND] = "BITAND",
    [SOL_TK_SYM_BITNOT] = "BITNOT",
    [SOL_TK_SYM_BITOR] = "BITOR",
    [SOL_TK_SYM_LEN] = "LEN",
    [SOL_TK_SYM_LESS] = "LESS",
    [SOL_TK_SYM_MORE] = "MORE",
    [SOL_TK_SYM_LPAREN] = "LPAREN",
    [SOL_TK_SYM_RPAREN] = "RPAREN",
    [SOL_TK_SYM_LBRACKET] = "LBRACKET",
    [SOL_TK_SYM_RBRACKET] = "RBRACKET",
    [SOL_TK_SYM_LBRACE] = "LBRACE",
    [SOL_TK_SYM_RBRACE] = "RBRACE",
    [SOL_TK_SYM_ASSIGN] = "ASSIGN",
    [SOL_TK_SYM_SEMICOLON] = "SEMICOLON",
    [SOL_TK_SYM_COLON] = "COLON",
    [SOL_TK_SYM_COMMA] = "COMMA",
    [SOL_TK_SYM_PERIOD] = "PERIOD",

    [SOL_TK_KW_FUNCTION] = "FUNCTION",
    [SOL_TK_KW_REPEAT] = "REPEAT",
    [SOL_TK_KW_ELSEIF] = "ELSEIF",
    [SOL_TK_KW_RETURN] = "RETURN",
    [SOL_TK_KW_BREAK] = "BREAK",
    [SOL_TK_KW_FALSE] = "FALSE",
    [SOL_TK_KW_LOCAL] = "LOCAL",
    [SOL_TK_KW_UNTIL] = "UNTIL",
    [SOL_TK_KW_WHILE] = "WHILE",
    [SOL_TK_KW_ELSE] = "ELSE",
    [SOL_TK_KW_GOTO] = "GOTO",
    [SOL_TK_KW_THEN] = "THEN",
    [SOL_TK_KW_TRUE] = "TRUE",
    [SOL_TK_KW_AND] = "AND",
    [SOL_TK_KW_END] = "END",
    [SOL_TK_KW_FOR] = "FOR",
    [SOL_TK_KW_NIL] = "NIL",
    [SOL_TK_KW_NOT] = "NOT",
    [SOL_TK_KW_DO] = "DO",
    [SOL_TK_KW_IF] = "IF",
    [SOL_TK_KW_IN] = "IN",
};
uint64 tstrtable_len = (sizeof tstrtable / sizeof tstrtable[0]);

char *sol_ttype_to_str(TokenType type) {
  if (type >= tstrtable_len)
    return "UNKNOWN";
  return (char *)tstrtable[type];
}
