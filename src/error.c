#include "sol/error.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void sol_fatal(const char *restrict fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  exit(1);
}

void sol_fatal_internal(const char *restrict fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  printf("\n\n============\n");
  printf("You've encountered an internal error, meaning it could've happened "
         "because of a "
         "memory issue, faulty code in sol's side, etc.\n\nWorry not, as "
         "this is not your "
         "fault (even if, sadly, you can't fix it). Report it "
         "immediately so it doesn't happen again at:\n- Nykenik24@proton.me\n- "
         "https://github.com/Nykenik24/sol/issues\n");

  exit(1);
}
