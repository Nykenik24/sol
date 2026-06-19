#include "string.h"

#include <stdlib.h>
#include <string.h>

#define STRING_INITIAL_CAPACITY ((ulong)16)
#define STRING_GROWTH_FACTOR 2

static int ensure_capacity(string_t *string, ulong min_capacity) {
  if (min_capacity <= string->capacity)
    return 1;

  ulong new_capacity = string->capacity * STRING_GROWTH_FACTOR;
  if (new_capacity < min_capacity)
    new_capacity = min_capacity;

  char *new_data = (char *)realloc(string->data, new_capacity);
  if (!new_data)
    return 0;

  string->data = new_data;
  string->capacity = new_capacity;
  return 1;
}

string_t *new_str() {
  string_t *string = (string_t *)malloc(sizeof(string_t));
  if (!string)
    return NULL;

  string->data = (char *)malloc(STRING_INITIAL_CAPACITY);
  if (!string->data) {
    free(string);
    return NULL;
  }

  string->data[0] = '\0';
  string->len = 0;
  string->capacity = STRING_INITIAL_CAPACITY;
  return string;
}

void str_putc(string_t *string, const char c) {
  if (!string)
    return;

  if (!ensure_capacity(string, string->len + 2))
    return;

  string->data[string->len] = c;
  string->len++;
  string->data[string->len] = '\0';
}

void str_concat(string_t *string, const char *str) {
  if (!string || !str)
    return;

  ulong str_length = (ulong)strlen(str);
  if (str_length == 0)
    return;

  if (!ensure_capacity(string, string->len + str_length + 1))
    return;

  memcpy(string->data + string->len, str, str_length);
  string->len += str_length;
  string->data[string->len] = '\0';
}

string_t *str_dup(string_t *string) {
  if (!string)
    return NULL;

  string_t *copy = (string_t *)malloc(sizeof(string_t));
  if (!copy)
    return NULL;

  copy->data = (char *)malloc(string->capacity);
  if (!copy->data) {
    free(copy);
    return NULL;
  }

  memcpy(copy->data, string->data, string->len + 1);
  copy->len = string->len;
  copy->capacity = string->capacity;
  return copy;
}

ulong str_len(string_t *string) {
  if (!string)
    return 0;

  return string->len;
}

const char *str_cstr(string_t *string) {
  if (!string)
    return NULL;

  return string->data;
}

void free_str(string_t *string) {
  if (!string)
    return;

  free(string->data);
  free(string);
}
