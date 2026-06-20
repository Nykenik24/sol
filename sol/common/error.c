#include "error.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define COLOR_RED ""
#define COLOR_YELLOW ""
#define COLOR_BLUE ""
#define COLOR_MAGENTA ""
#define COLOR_BOLD ""
#define COLOR_RESET ""
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_GRAY "\x1b[90m"
#define COLOR_BOLD "\x1b[1m"
#define COLOR_RESET "\x1b[0m"
#include <unistd.h>
#endif

error_info_t line_n_col(ulong line, ulong col) {
  error_info_t info = {
      .line = line,
      .col = col,
  };
  return info;
}

static const char *diag_name(diag_kind_t kind) {
  switch (kind) {
  case SOL_DIAG_ERROR:
    return "error";

  case SOL_DIAG_WARNING:
    return "warning";

  case SOL_DIAG_NOTE:
    return "note";

  case SOL_DIAG_ICE:
    return "internal compiler error";

  case SOL_DIAG_WIP:
    return "work-in-progress";
  }

  return "diagnostic";
}

static const char *diag_color(diag_kind_t kind) {
  switch (kind) {
  case SOL_DIAG_ERROR:
    return COLOR_RED;

  case SOL_DIAG_WARNING:
    return COLOR_YELLOW;

  case SOL_DIAG_NOTE:
    return COLOR_BLUE;

  case SOL_DIAG_ICE:
    return COLOR_MAGENTA;

  case SOL_DIAG_WIP:
    return COLOR_GREEN;
  }

  return "";
}

bool sol_colors_enabled(void) {
#ifdef _WIN32
  return false;
#else
  return isatty(fileno(stderr));
#endif
}

static void print_header(diag_kind_t kind) {
  if (sol_colors_enabled()) {
    fprintf(stderr, "%s%s%s%s: ", COLOR_BOLD, diag_color(kind), diag_name(kind),
            COLOR_RESET);
  } else {
    fprintf(stderr, "%s: ", diag_name(kind));
  }
}

void sol_diag(diag_kind_t kind, const error_info_t *info, const char *fmt,
              ...) {
  print_header(kind);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  if (info) {
    bool has_pos = info->file || info->line || info->col;

    if (has_pos) {
      if (sol_colors_enabled()) {
        fprintf(stderr, COLOR_GRAY "at " COLOR_RESET);

        if (info->file)
          fprintf(stderr, "%s", info->file);

        if (info->line) {
          fprintf(stderr, "%s" COLOR_MAGENTA "%lu" COLOR_RESET,
                  info->file ? ":" : "", info->line);

          if (info->col)
            fprintf(stderr, ":" COLOR_MAGENTA "%lu" COLOR_RESET, info->col);
        }

        fputc('\n', stderr);
      } else {
        fprintf(stderr, "at ");

        if (info->file)
          fprintf(stderr, "%s", info->file);

        if (info->line) {
          fprintf(stderr, "%s%lu", info->file ? ":" : "", info->line);

          if (info->col)
            fprintf(stderr, ":%lu", info->col);
        }

        fputc('\n', stderr);
      }
    }

    if (info->source_line) {
      fprintf(stderr, "  |\n");

      if (info->line)
        fprintf(stderr, "%lu | %s\n", info->line, info->source_line);
      else
        fprintf(stderr, "  | %s\n", info->source_line);

      if (info->col) {
        fprintf(stderr, "  | ");

        for (ulong i = 1; i < info->col; i++)
          fputc(' ', stderr);

        if (sol_colors_enabled()) {
          fprintf(stderr, "%s^%s\n", diag_color(kind), COLOR_RESET);
        } else {
          fputc('^', stderr);
          fputc('\n', stderr);
        }
      }
    }
  }

  if (kind == SOL_DIAG_ERROR || kind == SOL_DIAG_ICE || kind == SOL_DIAG_WIP) {
    if (kind == SOL_DIAG_ICE) {
      fprintf(stderr, "\nPlease report this bug:\n"
                      "  https://github.com/Nykenik24/sol/issues\n");
    }

    exit(EXIT_FAILURE);
  }
}
