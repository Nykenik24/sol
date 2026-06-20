#ifndef _INCLUDE_COMMON_HASHMAP_H_
#define _INCLUDE_COMMON_HASHMAP_H_

#include <stddef.h>

typedef struct {
  char *key;   // owned copy of the key, NULL if slot is empty
  void *value; // not owned by the map
} hashmap_entry_t;

typedef struct {
  hashmap_entry_t *entries;
  size_t capacity; // number of slots (always a power of two)
  size_t count;    // number of occupied slots
} hashmap_t;

// creates a map
hashmap_t *new_map();
// pushes a pair into the map
void map_push(hashmap_t *map, const char *key, void *value);
// returns a value of the map by key
void *map_get(hashmap_t *map, const char *key);
// frees a map
void free_map(hashmap_t *map);

#endif // !_INCLUDE_COMMON_HASHMAP_H_
