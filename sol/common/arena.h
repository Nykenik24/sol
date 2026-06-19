#ifndef _INCLUDE_COMMON_ARENA_H_
#define _INCLUDE_COMMON_ARENA_H_

#include "types.h"

typedef struct arena_block_t {
  struct arena_block_t *next;
  ulong size;   // total usable bytes in this block's data area
  ulong offset; // bump pointer, in bytes, from the start of data
  uchar *data;
} arena_block_t;

typedef struct {
  arena_block_t *head;   // most recently allocated block (bump-allocates here)
  ulong last_block_size; // size of the most recently allocated block, for
                         // geometric growth
} arena_t;

// creates a new arena
arena_t *new_arena();
// allocates nbytes of space in the arena, returns the pointer
void *arena_alloc(arena_t *arena, ulong nbytes);
// frees an arena
void free_arena(arena_t *arena);

#endif // !_INCLUDE_COMMON_ARENA_H_
