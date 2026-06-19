#ifndef _INCLUDE_COMMON_VECTOR_H_
#define _INCLUDE_COMMON_VECTOR_H_

#include "types.h"
typedef struct {
  uchar *data;
  ulong element_size;
  ulong size;     // number of elements currently stored
  ulong capacity; // number of elements the buffer can hold
} vector_t;

// creates a new vector
vector_t *new_vector(ulong element_size);
// copies element_size bytes from *element
void push_back(vector_t *vector, void *element);
// returns pointer to element in-place
void *vector_get(vector_t *vector, ulong index);
// returns the number of elements in the vector
ulong vector_size(vector_t *vector);
// frees a vector
void free_vector(vector_t *vector);

#endif // !_INCLUDE_COMMON_VECTOR_H_
