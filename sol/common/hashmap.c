#include "hashmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_INITIAL_CAPACITY 16
#define HASHMAP_LOAD_FACTOR_NUM 3
#define HASHMAP_LOAD_FACTOR_DEN 4

static uint64_t fnv1a_hash(const char *key) {
  uint64_t hash = 0xcbf29ce484222325ULL;
  for (const unsigned char *p = (const unsigned char *)key; *p; p++) {
    hash ^= (uint64_t)*p;
    hash *= 0x100000001b3ULL;
  }
  return hash;
}

static hashmap_entry_t *find_slot(hashmap_entry_t *entries, size_t capacity,
                                  const char *key) {
  uint64_t hash = fnv1a_hash(key);
  size_t index = (size_t)(hash & (capacity - 1));

  for (;;) {
    hashmap_entry_t *entry = &entries[index];
    if (entry->key == NULL || strcmp(entry->key, key) == 0) {
      return entry;
    }
    index = (index + 1) & (capacity - 1);
  }
}

static void map_grow(hashmap_t *map) {
  size_t new_capacity = map->capacity * 2;
  hashmap_entry_t *new_entries = calloc(new_capacity, sizeof(hashmap_entry_t));
  if (new_entries == NULL) {
    return;
  }

  for (size_t i = 0; i < map->capacity; i++) {
    hashmap_entry_t *old_entry = &map->entries[i];
    if (old_entry->key == NULL) {
      continue;
    }
    hashmap_entry_t *slot =
        find_slot(new_entries, new_capacity, old_entry->key);
    slot->key = old_entry->key;
    slot->value = old_entry->value;
  }

  free(map->entries);
  map->entries = new_entries;
  map->capacity = new_capacity;
}

hashmap_t *new_map() {
  hashmap_t *map = malloc(sizeof(hashmap_t));
  if (map == NULL) {
    return NULL;
  }

  map->capacity = HASHMAP_INITIAL_CAPACITY;
  map->count = 0;
  map->entries = calloc(map->capacity, sizeof(hashmap_entry_t));
  if (map->entries == NULL) {
    free(map);
    return NULL;
  }

  return map;
}

void map_push(hashmap_t *map, const char *key, void *value) {
  if (map == NULL || key == NULL) {
    return;
  }

  if ((map->count + 1) * HASHMAP_LOAD_FACTOR_DEN >
      map->capacity * HASHMAP_LOAD_FACTOR_NUM) {
    map_grow(map);
  }

  hashmap_entry_t *slot = find_slot(map->entries, map->capacity, key);
  if (slot->key != NULL) {
    slot->value = value;
    return;
  }

  slot->key = strdup(key);
  slot->value = value;
  map->count++;
}

void *map_get(hashmap_t *map, const char *key) {
  if (map == NULL || key == NULL || map->count == 0) {
    return NULL;
  }

  hashmap_entry_t *slot = find_slot(map->entries, map->capacity, key);
  if (slot->key == NULL) {
    return NULL;
  }
  return slot->value;
}

void free_map(hashmap_t *map) {
  if (map == NULL) {
    return;
  }

  for (size_t i = 0; i < map->capacity; i++) {
    free(map->entries[i].key);
  }
  free(map->entries);
  free(map);
}
