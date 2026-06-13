#include "pluja/lexer/lexer.h"
#include "pluja/buffer.h"
#include "pluja/error.h"
#include "pluja/lexer/token.h"
#include "pluja/types.h"
#include <stdlib.h>
#include <string.h>

void plj_cleanup_lex_res(token_t **tokens, uint64 tk_num) {
  for (idx_t i = 0; i < tk_num; i++) {
    plj_token_destroy(tokens[i]);
  }
}

static bool is_alpha(uint8 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool is_num(uint8 c) { return (c >= '0' && c <= '9'); }

static bool is_ws(uint8 c) {
  return (c == '\n' || c == ' ' || c == '\t' || c == '\r');
}

char *symtable[] = {"...", "..", "==", "<=", ">=", "+", "-", "*", "/",
                    "%",   "^",  "#",  "<",  ">",  "(", ")", "[", "]",
                    "{",   "}",  "=",  ";",  ":",  ",", "."};

char *kwtable[] = {"function", "repeat", "elseif", "return", "break", "false",
                   "local",    "until",  "while",  "else",   "goto",  "then",
                   "true",     "and",    "end",    "for",    "nil",   "not",
                   "do",       "if",     "in"};

typedef struct {
  uint64 cap;
  uint64 num;
  token_t **list;
} token_list;

token_list *tklist_init() {
  token_list *tklist = malloc(sizeof(token_list));

  tklist->cap = 8;
  tklist->num = 0;
  tklist->list = calloc(tklist->cap, sizeof(token_t));

  return tklist;
}

void tklist_push(token_list *tklist, token_t *tk) {
  if (tklist->num >= tklist->cap) {
    tklist->cap *= 2;
    tklist->list = realloc(tklist->list, sizeof(token_t) * tklist->cap);
  }
  tklist->list[tklist->num++] = tk;
}

#define INBOUNDS(input) ((i <= strlen(input)) && (i >= 0))
#define CAN_PUTC(input) ((i + 1 <= strlen(input)) && (i + 1 >= 0))

#define UNTERMINATED_STRING_ERROR                                              \
  {                                                                            \
    char *unterminated = plj_buffer_destroy(buf, NULL);                        \
    plj_fatal("unterminated string at line %lu near %c%s\n", line, term,       \
              unterminated);                                                   \
  }

token_t **plj_lex(const char *input, uint64 *tk_num) {
  token_list *tokens = tklist_init();

  uint64 line = 1;
  for (idx_t i = 0; i < strlen(input); i++) {
    if (is_ws(input[i])) {
      if (input[i] == '\n')
        line++;
      continue;
    }

    if (is_num(input[i])) {
      buffer_t *buf = plj_buffer_new(16);

      while (CAN_PUTC(input) && is_num(input[i])) {
        plj_buffer_putc(buf, input[i]);
        i++;
      }

      if (CAN_PUTC(input)) {
        if (input[i] != '.')
          goto push;

        plj_buffer_putc(buf, input[i]);

        i++;
        if (!INBOUNDS(input))
          goto push;

        while (CAN_PUTC(input) && is_num(input[i])) {
          plj_buffer_putc(buf, input[i]);
          i++;
        }
      } else
        goto push;

    push: {
      uint64 len;
      char *txt = plj_buffer_destroy(buf, &len);
      if (!txt)
        plj_fatal_internal("duplication of buffer for digit returned NULL\n");
      token_t *tk = plj_token_create(txt, len, PLJ_TK_DIGIT);
      tklist_push(tokens, tk);
      continue;
    }
    }

    if (input[i] == '"' || input[i] == '\'') {
      uint8 term = input[i];
      i++;

      if (!INBOUNDS(input))
        plj_fatal("unterminated string at line %lu near %c\n", line, term);

      buffer_t *buf = plj_buffer_new(16);
      while (CAN_PUTC(input) && input[i] != term) {
        plj_buffer_putc(buf, input[i]);
        i++;
      }
      if (input[i] == term)
        i++;
      else
        UNTERMINATED_STRING_ERROR

      if (!INBOUNDS(input))
        UNTERMINATED_STRING_ERROR;

      uint64 len;
      char *txt = plj_buffer_destroy(buf, &len);
      if (!txt)
        plj_fatal_internal("duplication of buffer for string returned NULL\n");
      token_t *tk = plj_token_create(txt, len, PLJ_TK_STRING);
      tklist_push(tokens, tk);
      continue;
    }

    for (idx_t j = 0; j < (sizeof kwtable / sizeof kwtable[0]); j++) {
      char *kw = kwtable[j];
      uint64 len = strlen(kw);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (idx_t k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(kw, buf) == 0) {
          token_t *tk = plj_token_create(kw, len, PLJ_TK_RESERVED);
          tklist_push(tokens, tk);
          i += len;
          goto continue_;
        }
      }
    }

    if (is_alpha(input[i]) || input[i] == '_') {
      buffer_t *buf = plj_buffer_new(16);
      plj_buffer_putc(buf, input[i]);

      if (CAN_PUTC(input)) {
        i++;
      } else {
        char buf[2] = {input[i], '\0'};
        token_t *tk = plj_token_create(strdup(buf), 2, PLJ_TK_IDENT);
        tklist_push(tokens, tk);
        continue;
      }

      while (CAN_PUTC(input) &&
             (is_alpha(input[i]) || is_num(input[i]) || input[i] == '_')) {
        plj_buffer_putc(buf, input[i]);
        i++;
      }

      uint64 len;
      char *txt = plj_buffer_destroy(buf, &len);
      if (!txt)
        plj_fatal_internal(
            "duplication of buffer for identifier returns NULL\n");
      token_t *tk = plj_token_create(txt, len, PLJ_TK_IDENT);
      tklist_push(tokens, tk);
      continue;
    }

    for (idx_t j = 0; j < (sizeof symtable / sizeof symtable[0]); j++) {
      char *sym = symtable[j];
      uint64 len = strlen(sym);
      if ((i + len <= strlen(input)) && (i + len >= 0)) {
        char buf[len + 1];
        for (idx_t k = 0; k < len; k++) {
          buf[k] = input[i + k];
        }
        buf[len] = '\0';
        if (strcmp(sym, buf) == 0) {
          token_t *tk = plj_token_create(sym, len, PLJ_TK_SYMBOL);
          tklist_push(tokens, tk);
          i += len;
          goto continue_;
        }
      }
    }

    plj_fatal("unrecognized character '%c' in line %lu\n", input[i], line);

  continue_: {}
  }

  tklist_push(tokens, plj_token_eof());

  if (tk_num)
    *tk_num = tokens->num;
  return tokens->list;
}
