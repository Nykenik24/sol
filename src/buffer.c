#include "sol/mem.h"
#include <stdlib.h>
#include <string.h>

Buffer *sol_buffer_new(uint64 start_cap) {
  Buffer *buf = malloc(sizeof(Buffer));
  buf->cap = start_cap;
  buf->num = 0;
  buf->buf = malloc(start_cap);
  return buf;
};

void sol_buffer_putc(Buffer *buf, uint8 c) {
  if (buf->num >= buf->cap) {
    buf->cap *= 2;
    buf->buf = realloc(buf->buf, sizeof(char) * buf->cap);
  }
  buf->buf[buf->num++] = c;
}

void sol_buffer_puts(Buffer *buf, const char *str) {
  uint64 len = strlen(str);
  if (buf->num + len >= buf->cap) {
    buf->cap += (len + 1);
    buf->buf = realloc(buf->buf, sizeof(char) * buf->cap);
  }

  for (size i = 0; i < len; i++) {
    buf->buf[buf->num++] = str[i];
  }
}

char *sol_buffer_destroy(Buffer *buf, uint64 *buf_size) {
  if (!buf)
    return NULL;

  if (buf->buf) {
    if (buf_size)
      *buf_size = buf->num;

    char *out = malloc(buf->num + 1);
    memcpy(out, buf->buf, buf->num);
    out[buf->num] = '\0';

    free(buf->buf);
    free(buf);
    return out;
  }

  free(buf);
  return NULL;
}
