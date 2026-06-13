#ifndef PLUJA_INCLUDE__ERROR_H
#define PLUJA_INCLUDE__ERROR_H

// Print and exit with code 1.
void plj_fatal(const char *restrict fmt, ...);

// Same as plj_fatal, but has some additional information
// about, well, the fact it's internal.
void plj_fatal_internal(const char *restrict fmt, ...);

#endif // !PLUJA_INCLUDE__ERROR_H
