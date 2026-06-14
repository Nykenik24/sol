#include "pluja/mem.h"
#include "pluja/types.h"
#include <stdlib.h>

list_t *plj_list_init() {
  list_t *list = malloc(sizeof(list_t));

  list->cap = 16;
  list->num = 0;
  list->raw = calloc(list->cap, sizeof(void *));

  return list;
}

#define LIST_OOM(LIST) (LIST->num >= LIST->cap)

void plj_list_push(list_t *list, void *ptr) {
  if (LIST_OOM(list)) {
    while (LIST_OOM(list)) {
      list->cap *= 2;
    }
    list->raw = realloc(list->raw, list->cap * sizeof(void *));
  }
  list->raw[list->num++] = ptr;
}

void plj_list_free(list_t *list) {
  if (!list)
    return;

  for (size i = 0; i < list->num; i++) {
    if (list->raw[i])
      free(list->raw[i]);
  }

  free(list);
}
