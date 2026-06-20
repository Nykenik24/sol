#ifndef _INCLUDE_COMMON_ERROR_H_
#define _INCLUDE_COMMON_ERROR_H_

#include "types.h"

typedef enum {
  SOL_DIAG_ERROR,
  SOL_DIAG_WARNING,
  SOL_DIAG_NOTE,
  SOL_DIAG_ICE,
  SOL_DIAG_WIP,
} diag_kind_t;

typedef struct {
  const char *file;

  ulong line;
  ulong col;

  const char *source_line;

  bool has_file;
  bool has_location;
  bool has_source;
} error_info_t;

error_info_t line_n_col(ulong line, ulong col);
void sol_diag(diag_kind_t kind, const error_info_t *info, const char *fmt, ...);

#endif // !_INCLUDE_COMMON_ERROR_H_
