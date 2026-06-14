#include "sol/mem.h"
#include "sol/types.h"
#include <stdlib.h>

List *sol_list_init() {
  List *list = malloc(sizeof(List));

  list->cap = 16;
  list->num = 0;
  list->raw = calloc(list->cap, sizeof(void *));

  return list;
}

#define LIST_OOM(LIST) (LIST->num >= LIST->cap)

void sol_list_push(List *list, void *ptr) {
  if (LIST_OOM(list)) {
    while (LIST_OOM(list)) {
      list->cap *= 2;
    }
    list->raw = realloc(list->raw, list->cap * sizeof(void *));
  }
  list->raw[list->num++] = ptr;
}

void sol_list_free(List *list) {
  if (!list)
    return;

  for (size i = 0; i < list->num; i++) {
    if (list->raw[i])
      free(list->raw[i]);
  }

  free(list);
}
