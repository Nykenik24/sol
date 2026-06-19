#ifndef _INCLUDE_COMMON_ERROR_H_
#define _INCLUDE_COMMON_ERROR_H_

// Print and exit with code 1.
void sol_fatal(const char *restrict fmt, ...);

// Same as sol_fatal, but has some additional information
// about, well, the fact it's internal.
void sol_fatal_internal(const char *restrict fmt, ...);

#endif // !_INCLUDE_COMMON_ERROR_H_
