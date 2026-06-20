#include "slexer.h"
#include "common/arena.h"
#include "common/error.h"
#include "common/string.h"
#include "common/types.h"
#include "common/vector.h"
#include <stdlib.h>
#include <string.h>

static bool is_alpha(uchar c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_num(uchar c) { return (c >= '0' && c <= '9'); }

static bool is_ws(uchar c) {
  return (c == '\n' || c == ' ' || c == '\t' || c == '\r');
}

static bool is_hex(uchar c) {
  return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || is_num(c);
}

typedef struct {
  token_type_t tt;
  char *txt;
} mapped_t;

mapped_t symtable[] = {
    {SOL_TK_SYM_VARARG, "..."},   {SOL_TK_SYM_CONCAT, ".."},
    {SOL_TK_SYM_LABEL, "::"},     {SOL_TK_SYM_EQUAL, "=="},
    {SOL_TK_SYM_LEFTSHIFT, "<<"}, {SOL_TK_SYM_RIGHTSHIFT, ">>"},
    {SOL_TK_SYM_NEQUAL, "~="},    {SOL_TK_SYM_LEQUAL, "<="},
    {SOL_TK_SYM_GEQUAL, ">="},    {SOL_TK_SYM_ADD, "+"},
    {SOL_TK_SYM_SUB, "-"},        {SOL_TK_SYM_MUL, "*"},
    {SOL_TK_SYM_DIV, "/"},        {SOL_TK_SYM_MOD, "%"},
    {SOL_TK_SYM_BITXOR, "^"},     {SOL_TK_SYM_BITAND, "&"},
    {SOL_TK_SYM_BITNOT, "~"},     {SOL_TK_SYM_BITOR, "|"},
    {SOL_TK_SYM_LEN, "#"},        {SOL_TK_SYM_LESS, "<"},
    {SOL_TK_SYM_MORE, ">"},       {SOL_TK_SYM_LPAREN, "("},
    {SOL_TK_SYM_RPAREN, ")"},     {SOL_TK_SYM_LBRACKET, "["},
    {SOL_TK_SYM_RBRACKET, "]"},   {SOL_TK_SYM_LBRACE, "{"},
    {SOL_TK_SYM_RBRACE, "}"},     {SOL_TK_SYM_ASSIGN, "="},
    {SOL_TK_SYM_SEMICOLON, ";"},  {SOL_TK_SYM_COLON, ":"},
    {SOL_TK_SYM_COMMA, ","},      {SOL_TK_SYM_PERIOD, "."},
};

mapped_t kwtable[] = {
    {SOL_TK_KW_FUNCTION, "function"},
    {SOL_TK_KW_EXPORT, "export"},
    {SOL_TK_KW_REPEAT, "repeat"},
    {SOL_TK_KW_ELSEIF, "elseif"},
    {SOL_TK_KW_RETURN, "return"},
    {SOL_TK_KW_BREAK, "break"},
    {SOL_TK_KW_FALSE, "false"},
    {SOL_TK_KW_UNTIL, "until"},
    {SOL_TK_KW_WHILE, "while"},
    {SOL_TK_KW_ELSE, "else"},
    {SOL_TK_KW_GOTO, "goto"},
    {SOL_TK_KW_THEN, "then"},
    {SOL_TK_KW_TRUE, "true"},
    {SOL_TK_KW_EACH, "each"},
    {SOL_TK_KW_VAR, "var"},
    {SOL_TK_KW_AND, "and"},
    {SOL_TK_KW_END, "end"},
    {SOL_TK_KW_NIL, "nil"},
    {SOL_TK_KW_NOT, "not"},
    {SOL_TK_KW_DO, "do"},
    {SOL_TK_KW_IF, "if"},
    {SOL_TK_KW_IN, "in"},
    {SOL_TK_KW_OR, "or"},
};

#define INBOUNDS(input) (i <= strlen(input))
#define CAN_PUTC(input) (i + 1 <= strlen(input))

lexer_t *new_lexer() {
  lexer_t *lexer = (lexer_t *)malloc(sizeof(lexer_t));
  lexer->arena = new_arena();
  lexer->tokens = new_vector(sizeof(token_t));
  return lexer;
}

void free_lexer(lexer_t *lexer) {
  if (!lexer)
    return;
  free_arena(lexer->arena);
  free_vector(lexer->tokens);

  free(lexer);
}

static char *arena_strdup(arena_t *arena, const char *s) {
  size_t len = strlen(s) + 1;
  char *copy = (char *)arena_alloc(arena, len);
  memcpy(copy, s, len);
  return copy;
}

static token_t *new_token(lexer_t *lexer, const char *txt, token_type_t type) {
  token_t *tk = (token_t *)arena_alloc(lexer->arena, sizeof(token_t));
  tk->txt = arena_strdup(lexer->arena, txt);
  tk->type = type;
  return tk;
}

static void push_token(lexer_t *lexer, token_t *tk) {
  push_back(lexer->tokens, tk);
}

#define next                                                                   \
  i++;                                                                         \
  col++
#define skip(n)                                                                \
  i += (n);                                                                    \
  col += (n)

void lex(lexer_t *lexer, const char *input) {
  lexer->tokens = new_vector(sizeof(token_t));

  ulong line = 1;
  ulong col = 1;
  ulong i = 0;
  while (i < strlen(input)) {
    while (is_ws(input[i])) {
      if (input[i] == '\n') {
        line++;
        col = 1;
      } else {
        col++;
      }

      i++;
    }

    if (input[i] == '-' && CAN_PUTC(input) && input[i + 1] == '-') {
      skip(2);
      while (INBOUNDS(input) && (input[i] != '\n')) {
        next;
      }
      next;
      line++;
      continue;
    }

    if (i >= strlen(input))
      break;

    if (input[i] == '[' && CAN_PUTC(input) && input[i + 1] == '[') {
      skip(2);
      if (!CAN_PUTC(input)) {
        error_info_t info = line_n_col(line, col);
        sol_diag(SOL_DIAG_ERROR, &info, "unstarted multi-line string\n");
      }

      string_t *buf = new_str();
      while (input[i] != ']') {
        if (input[i] == '\\') {
          next;

          if (!CAN_PUTC(input)) {
            error_info_t info = line_n_col(line, col);
            sol_diag(SOL_DIAG_ERROR, &info,
                     "unterminated multi-line string near [[%s\n",
                     str_cstr(buf));
          }

          str_putc(buf, input[i]);
          next;
        }

        next;
        if (!CAN_PUTC(input)) {
          error_info_t info = line_n_col(line, col);
          sol_diag(SOL_DIAG_ERROR, &info,
                   "unterminated multi-line string near [[%s\n", str_cstr(buf));
        }

        if (input[i] != ']') {
          error_info_t info = line_n_col(line, col);
          sol_diag(SOL_DIAG_ERROR, &info,
                   "wrong terminator for multi-line string\n");
        }

        next;

        ulong len;
        token_t *tk = new_token(lexer, str_cstr(buf), SOL_TK_STRING);
        tk->line = line;
        push_back(lexer->tokens, tk);
        continue;
      }
    }

    for (ulong j = 0; j < (sizeof symtable / sizeof symtable[0]); j++) {
      mapped_t sym = symtable[j];
      ulong len = strlen(sym.txt);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (ulong k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(sym.txt, buf) == 0) {
          token_t *tk = new_token(lexer, sym.txt, sym.tt);
          tk->line = line;
          tk->col = col;
          skip(len);
          push_token(lexer, tk);
          goto continue_;
        }
      }
    }

    for (ulong j = 0; j < (sizeof kwtable / sizeof kwtable[0]); j++) {
      mapped_t kw = kwtable[j];
      ulong len = strlen(kw.txt);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (ulong k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(kw.txt, buf) == 0) {
          char nxt = input[i + len];
          if (is_alpha(nxt) || is_num(nxt) || nxt == '_')
            continue;
          token_t *tk = new_token(lexer, kw.txt, kw.tt);
          tk->line = line;
          tk->col = col;
          skip(len);
          push_token(lexer, tk);
          goto continue_;
        }
      }
    }

    if (input[i] == '0' && CAN_PUTC(input) && input[i + 1] == 'x') {
      i += 2;
      if (!CAN_PUTC(input)) {
        error_info_t info = line_n_col(line, col);
        sol_diag(SOL_DIAG_ERROR, &info, "unstarted hex literal\n");
      }

      string_t *buf = new_str();
      while (i < strlen(input) && is_hex(input[i])) {
        str_putc(buf, input[i]);
        next;
      }

      token_t *tk = new_token(lexer, str_cstr(buf), SOL_TK_HEX_DIGIT);
      tk->line = line;
      tk->col = col;
      push_token(lexer, tk);
      continue;
    }

    if (is_num(input[i])) {
      string_t *buf = new_str();
      token_type_t kind = SOL_TK_INT;

      while (CAN_PUTC(input) && is_num(input[i])) {
        str_putc(buf, input[i]);
        next;
      }

      if (CAN_PUTC(input)) {
        if (input[i] != '.')
          goto push;

        kind = SOL_TK_FLOAT;
        str_putc(buf, input[i]);

        next;
        if (!INBOUNDS(input))
          goto push;

        while (CAN_PUTC(input) && is_num(input[i])) {
          str_putc(buf, input[i]);
          next;
        }
      } else
        goto push;

    push: {
      token_t *tk = new_token(lexer, str_cstr(buf), kind);
      tk->line = line;
      tk->col = col;
      push_token(lexer, tk);
      continue;
    }
    }

    if (input[i] == '"' || input[i] == '\'') {
      uchar term = input[i];
      next;

      if (!INBOUNDS(input)) {
        error_info_t info = line_n_col(line, col);
        sol_diag(SOL_DIAG_ERROR, &info, "unterminated string near %c\n", term);
      }

      string_t *buf = new_str();
      while (CAN_PUTC(input) && input[i] != term && input[i] != '\n') {
        str_putc(buf, input[i]);
        next;
      }
      if (input[i] == term) {
        next;
      } else {
        error_info_t info = line_n_col(line, col);
        sol_diag(SOL_DIAG_ERROR, &info, "unterminated string near %c\n", term);
      }

      if (!INBOUNDS(input)) {
        error_info_t info = line_n_col(line, col);
        sol_diag(SOL_DIAG_ERROR, &info, "unterminated string near %c\n", term);
      }

      token_t *tk = new_token(lexer, str_cstr(buf), SOL_TK_STRING);
      tk->line = line;
      tk->col = col;
      push_token(lexer, tk);
      continue;
    }

    if (is_alpha(input[i]) || input[i] == '_') {
      string_t *buf = new_str();
      str_putc(buf, input[i]);

      if (CAN_PUTC(input)) {
        next;
      } else {
        char s[2] = {input[i], '\0'};
        token_t *tk = new_token(lexer, strdup(s), SOL_TK_IDENT);
        tk->line = line;
        tk->col = col;
        push_token(lexer, tk);
        continue;
      }

      while (CAN_PUTC(input) &&
             (is_alpha(input[i]) || is_num(input[i]) || input[i] == '_')) {
        str_putc(buf, input[i]);
        next;
      }

      token_t *tk = new_token(lexer, str_cstr(buf), SOL_TK_IDENT);
      tk->line = line;
      tk->col = col;
      push_token(lexer, tk);
      continue;
    }

    error_info_t info = line_n_col(line, col);
    sol_diag(SOL_DIAG_ERROR, &info, "unrecognized character '%c'\n", input[i]);

  continue_: {}
  }

  char eofs[1] = {'\0'};
  token_t *eof = new_token(lexer, eofs, SOL_TK_EOF);
  eof->line = line;
  eof->col = col;
  push_token(lexer, eof);
}
