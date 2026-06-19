#include "vector.h"
#include "types.h"

#include <stdlib.h>
#include <string.h>

#define VECTOR_INITIAL_CAPACITY ((ulong)8)
#define VECTOR_GROWTH_FACTOR 2

vector_t *new_vector(ulong element_size) {
  if (element_size == 0)
    return NULL;

  vector_t *vector = (vector_t *)malloc(sizeof(vector_t));
  if (!vector)
    return NULL;

  vector->data = (uchar *)malloc(element_size * VECTOR_INITIAL_CAPACITY);
  if (!vector->data) {
    free(vector);
    return NULL;
  }

  vector->element_size = element_size;
  vector->size = 0;
  vector->capacity = VECTOR_INITIAL_CAPACITY;
  return vector;
}

void push_back(vector_t *vector, void *element) {
  if (!vector || !element)
    return;

  if (vector->size == vector->capacity) {
    ulong new_capacity = vector->capacity * VECTOR_GROWTH_FACTOR;
    uchar *new_data =
        (uchar *)realloc(vector->data, new_capacity * vector->element_size);
    if (!new_data)
      return;

    vector->data = new_data;
    vector->capacity = new_capacity;
  }

  uchar *dest = vector->data + (vector->size * vector->element_size);
  memcpy(dest, element, vector->element_size);
  vector->size++;
}

void *vector_get(vector_t *vector, ulong index) {
  if (!vector || index >= vector->size)
    return NULL;

  return vector->data + (index * vector->element_size);
}

ulong vector_size(vector_t *vector) {
  if (!vector)
    return 0;

  return vector->size;
}

void free_vector(vector_t *vector) {
  if (!vector)
    return;

  free(vector->data);
  free(vector);
}
