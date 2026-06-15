#include "sol/lexer/lexer.h"
#include "sol/error.h"
#include "sol/lexer/token.h"
#include "sol/mem.h"
#include "sol/types.h"
#include <string.h>

void sol_cleanup_lex_res(Token **tokens, uint64 tk_num) {
  for (size i = 0; i < tk_num; i++) {
    sol_token_destroy(tokens[i]);
  }
}

static bool is_alpha(uint8 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_num(uint8 c) { return (c >= '0' && c <= '9'); }

static bool is_ws(uint8 c) {
  return (c == '\n' || c == ' ' || c == '\t' || c == '\r');
}

static bool is_hex(uint8 c) {
  return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || is_num(c);
}

char *symtable[] = {
    "...", "..", "::", "==", "<<", ">>", "~=", "<=", ">=", "+", "-",
    "*",   "/",  "%",  "^",  "&",  "~",  "|",  "#",  "<",  ">", "(",
    ")",   "[",  "]",  "{",  "}",  "=",  ";",  ":",  ",",  "."};

char *kwtable[] = {"function", "repeat", "elseif", "return", "break", "false",
                   "local",    "until",  "while",  "else",   "goto",  "then",
                   "true",     "and",    "end",    "for",    "nil",   "not",
                   "do",       "if",     "in"};

#define INBOUNDS(input) ((i <= strlen(input)) && (i >= 0))
#define CAN_PUTC(input) ((i + 1 <= strlen(input)) && (i + 1 >= 0))

#define UNTERMINATED_STRING_ERROR                                              \
  {                                                                            \
    char *unterminated = sol_buffer_destroy(buf, NULL);                        \
    sol_fatal("unterminated string at line %lu near %c%s\n", line, term,       \
              unterminated);                                                   \
  }

#define UNTERMINATED_ML_STRING_ERROR                                           \
  {                                                                            \
    char *unterminated = sol_buffer_destroy(buf, NULL);                        \
    sol_fatal("unterminated multi-line string at line %lu near [[%s\n", line,  \
              unterminated);                                                   \
  }

Token **sol_lex(const char *input, uint64 *tk_num) {
  List *tokens = sol_list_create();

  uint64 line = 1;
  size i = 0;
  while (i < strlen(input)) {
    while (is_ws(input[i])) {
      if (input[i] == '\n')
        line++;
      i++;
    }

    if (input[i] == '-' && CAN_PUTC(input) && input[i + 1] == '-') {
      i += 2;
      while (INBOUNDS(input) && (input[i] != '\n')) {
        i++;
      }
      i++;
      line++;
      continue;
    }

    if (i >= strlen(input))
      break;

    if (input[i] == '[' && CAN_PUTC(input) && input[i + 1] == '[') {
      i += 2;
      if (!CAN_PUTC(input)) {
        sol_fatal("unstarted multi-line string at line %lu\n", line);
      }

      Buffer *buf = sol_buffer_new(16);
      while (input[i] != ']') {
        if (input[i] == '\\') {
          i++;

          if (!CAN_PUTC(input))
            UNTERMINATED_ML_STRING_ERROR
        }

        sol_buffer_putc(buf, input[i]);
        i++;
      }

      i++;
      if (!CAN_PUTC(input))
        UNTERMINATED_ML_STRING_ERROR

      if (input[i] != ']')
        sol_fatal("wrong terminator for multi-line string at line %lu\n", line);

      i++;

      uint64 len;
      char *txt = sol_buffer_destroy(buf, &len);
      if (!txt)
        sol_fatal_internal("duplication of buffer for string returned NULL\n");
      Token *tk = sol_token_create(txt, len, SOL_TK_STRING);
      tk->line = line;
      sol_list_push(tokens, tk);
      continue;
    }

    for (size j = 0; j < (sizeof symtable / sizeof symtable[0]); j++) {
      char *sym = symtable[j];
      uint64 len = strlen(sym);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (size k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(sym, buf) == 0) {
          Token *tk = sol_token_create(sym, len, SOL_TK_SYMBOL);
          tk->line = line;
          sol_list_push(tokens, tk);
          i += len;
          goto continue_;
        }
      }
    }

    for (size j = 0; j < (sizeof kwtable / sizeof kwtable[0]); j++) {
      char *kw = kwtable[j];
      uint64 len = strlen(kw);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (size k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(kw, buf) == 0) {
          char next = input[i + len];
          if (is_alpha(next) || is_num(next) || next == '_')
            continue;
          Token *tk = sol_token_create(kw, len, SOL_TK_RESERVED);
          tk->line = line;
          sol_list_push(tokens, tk);
          i += len;
          goto continue_;
        }
      }
    }

    if (input[i] == '0' && CAN_PUTC(input) && input[i + 1] == 'x') {
      i += 2;
      if (!CAN_PUTC(input))
        sol_fatal("unstarted hex literal at line %lu\n", line);

      Buffer *buf = sol_buffer_new(16);
      while (i < strlen(input) && is_hex(input[i])) {
        sol_buffer_putc(buf, input[i]);
        i++;
      }

      uint64 len;
      char *txt = sol_buffer_destroy(buf, &len);
      if (!txt)
        sol_fatal_internal(
            "duplication of buffer for hexadecimal digit returned NULL\n");
      Token *tk = sol_token_create(txt, len, SOL_TK_HEX_DIGIT);
      tk->line = line;
      sol_list_push(tokens, tk);
      continue;
    }

    if (is_num(input[i])) {
      Buffer *buf = sol_buffer_new(16);

      while (CAN_PUTC(input) && is_num(input[i])) {
        sol_buffer_putc(buf, input[i]);
        i++;
      }

      if (CAN_PUTC(input)) {
        if (input[i] != '.')
          goto push;

        sol_buffer_putc(buf, input[i]);

        i++;
        if (!INBOUNDS(input))
          goto push;

        while (CAN_PUTC(input) && is_num(input[i])) {
          sol_buffer_putc(buf, input[i]);
          i++;
        }
      } else
        goto push;

    push: {
      uint64 len;
      char *txt = sol_buffer_destroy(buf, &len);
      if (!txt)
        sol_fatal_internal("duplication of buffer for digit returned NULL\n");
      Token *tk = sol_token_create(txt, len, SOL_TK_DIGIT);
      tk->line = line;
      sol_list_push(tokens, tk);
      continue;
    }
    }

    if (input[i] == '"' || input[i] == '\'') {
      uint8 term = input[i];
      i++;

      if (!INBOUNDS(input))
        sol_fatal("unterminated string at line %lu near %c\n", line, term);

      Buffer *buf = sol_buffer_new(16);
      while (CAN_PUTC(input) && input[i] != term && input[i] != '\n') {
        sol_buffer_putc(buf, input[i]);
        i++;
      }
      if (input[i] == term)
        i++;
      else
        UNTERMINATED_STRING_ERROR

      if (!INBOUNDS(input))
        UNTERMINATED_STRING_ERROR;

      uint64 len;
      char *txt = sol_buffer_destroy(buf, &len);
      if (!txt)
        sol_fatal_internal("duplication of buffer for string returned NULL\n");
      Token *tk = sol_token_create(txt, len, SOL_TK_STRING);
      tk->line = line;
      sol_list_push(tokens, tk);
      continue;
    }

    if (is_alpha(input[i]) || input[i] == '_') {
      Buffer *buf = sol_buffer_new(16);
      sol_buffer_putc(buf, input[i]);

      if (CAN_PUTC(input)) {
        i++;
      } else {
        char buf[2] = {input[i], '\0'};
        Token *tk = sol_token_create(strdup(buf), 2, SOL_TK_IDENT);
        tk->line = line;
        sol_list_push(tokens, tk);
        continue;
      }

      while (CAN_PUTC(input) &&
             (is_alpha(input[i]) || is_num(input[i]) || input[i] == '_')) {
        sol_buffer_putc(buf, input[i]);
        i++;
      }

      uint64 len;
      char *txt = sol_buffer_destroy(buf, &len);
      if (!txt)
        sol_fatal_internal(
            "duplication of buffer for identifier returns NULL\n");
      Token *tk = sol_token_create(txt, len, SOL_TK_IDENT);
      tk->line = line;
      sol_list_push(tokens, tk);
      continue;
    }

    sol_fatal("unrecognized character '%c' in line %lu\n", input[i], line);

  continue_: {}
  }

  sol_list_push(tokens, sol_token_eof());

  if (tk_num)
    *tk_num = tokens->num;

  return (Token **)(tokens->raw);
}
