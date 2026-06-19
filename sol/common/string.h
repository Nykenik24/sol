#ifndef _INCLUDE_COMMON_STRING_H_
#define _INCLUDE_COMMON_STRING_H_

#include "types.h"
typedef struct {
  char *data;
  ulong len;      // number of characters, excluding the null terminator
  ulong capacity; // total buffer size, including room for the null terminator
} string_t;

// create a new string
string_t *new_str();
// append a character into a string
void str_putc(string_t *string, const char c);
// concatenate a string into another string
void str_concat(string_t *string, const char *str);
// duplicate a string
string_t *str_dup(string_t *string);
// get a number of characters in the string
ulong str_len(string_t *string);
// get a string as a const char*
const char *str_cstr(string_t *string);
// free a string
void free_str(string_t *string);

#endif // !_INCLUDE_COMMON_STRING_H_
