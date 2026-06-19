#include "arena.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_ALIGNMENT 16
#define ARENA_INITIAL_BLOCK_SIZE ((ulong)4096)
#define ARENA_GROWTH_FACTOR 2

typedef struct arena_block_t arena_block_t;

static inline ulong align_up(ulong n, ulong align) {
  return (n + (align - 1)) & ~(align - 1);
}

static arena_block_t *arena_block_new(ulong size) {
  arena_block_t *block = (arena_block_t *)malloc(sizeof(arena_block_t));
  if (!block)
    return NULL;

  block->data = (uchar *)malloc(size);
  if (!block->data) {
    free(block);
    return NULL;
  }

  block->next = NULL;
  block->size = size;
  block->offset = 0;
  return block;
}

arena_t *new_arena() {
  arena_t *arena = (arena_t *)malloc(sizeof(arena_t));
  if (!arena)
    return NULL;

  arena_block_t *block = arena_block_new(ARENA_INITIAL_BLOCK_SIZE);
  if (!block) {
    free(arena);
    return NULL;
  }

  arena->head = block;
  arena->last_block_size = ARENA_INITIAL_BLOCK_SIZE;
  return arena;
}

void *arena_alloc(arena_t *arena, ulong nbytes) {
  if (!arena || nbytes == 0)
    return NULL;

  arena_block_t *block = arena->head;
  ulong aligned_offset = align_up(block->offset, ARENA_ALIGNMENT);

  if (aligned_offset + nbytes > block->size) {
    // Current block can't fit this allocation; grow geometrically, but
    // make sure the new block is at least big enough for the request
    // (plus alignment slack) regardless of the growth factor.
    ulong new_size = arena->last_block_size * ARENA_GROWTH_FACTOR;
    ulong needed = nbytes + ARENA_ALIGNMENT;
    if (new_size < needed)
      new_size = needed;

    arena_block_t *new_block = arena_block_new(new_size);
    if (!new_block)
      return NULL;

    new_block->next = block;
    arena->head = new_block;
    arena->last_block_size = new_size;

    block = new_block;
    aligned_offset = 0;
  }

  void *ptr = block->data + aligned_offset;
  block->offset = aligned_offset + nbytes;
  return ptr;
}

void free_arena(arena_t *arena) {
  if (!arena)
    return;

  arena_block_t *block = arena->head;
  while (block) {
    arena_block_t *next = block->next;
    free(block->data);
    free(block);
    block = next;
  }

  free(arena);
}
