#ifndef SOL_INCLUDE__ERROR_H
#define SOL_INCLUDE__ERROR_H

// Print and exit with code 1.
void sol_fatal(const char *restrict fmt, ...);

// Same as sol_fatal, but has some additional information
// about, well, the fact it's internal.
void sol_fatal_internal(const char *restrict fmt, ...);

#endif // !SOL_INCLUDE__ERROR_H
