#ifndef SOL_INCLUDE__MEM_H
#define SOL_INCLUDE__MEM_H

#include "sol/types.h"

typedef struct {
  uint64 cap;
  uint64 num;
  char *buf;
} Buffer;

// Create a new char buffer.
Buffer *sol_buffer_new(uint64 start_cap);

// Put a char into a buffer.
void sol_buffer_putc(Buffer *buf, uint8 c);

// Put a string into a buffer.
void sol_buffer_puts(Buffer *buf, const char *str);

// Destroys the buffer object and returns a duplicate of
// the raw buffer.
//
// Pass an unsigned long by reference to buf_size to
// get the final buffer's size. It can be NULL if not
// needed.
char *sol_buffer_destroy(Buffer *buf, uint64 *buf_size);

typedef struct {
  uint64 cap;
  uint64 num;
  void **raw;
} List;

// Initialize/create a list.
List *sol_list_init();

// Push an elemnent into a list.
void sol_list_push(List *list, void *ptr);

// Free a list.
void sol_list_free(List *list);

// Clean-up an array

#endif // !SOL_INCLUDE__MEM_H
