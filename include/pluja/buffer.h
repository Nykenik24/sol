#ifndef PLUJA_INCLUDE__BUFFER_H
#define PLUJA_INCLUDE__BUFFER_H

#include "pluja/types.h"

typedef struct {
  uint64 cap;
  uint64 num;
  char *buf;
} buffer_t;

// Create a new char buffer.
buffer_t *plj_buffer_new(uint64 start_cap);

// Put a char into a buffer.
void plj_buffer_putc(buffer_t *buf, uint8 c);

// Put a string into a buffer.
void plj_buffer_puts(buffer_t *buf, const char *str);

// Destroys the buffer object and returns a duplicate of
// the raw buffer.
//
// Pass an unsigned long by reference to buf_size to
// get the final buffer's size. It can be NULL if not
// needed.
char *plj_buffer_destroy(buffer_t *buf, uint64 *buf_size);
#endif // !PLUJA_INCLUDE__BUFFER_H
