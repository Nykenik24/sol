#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sgen.h"
#include "slexer.h"
#include "sparser.h"

static int has_sun_extension(const char *path) {
  const char *dot = strrchr(path, '.');
  return dot != NULL && strcmp(dot, ".sun") == 0;
}

static char *read_file(const char *path, long *out_size) {
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "sol: could not open file '%s'\n", path);
    return NULL;
  }

  if (fseek(f, 0, SEEK_END) != 0) {
    fprintf(stderr, "sol: could not seek file '%s'\n", path);
    fclose(f);
    return NULL;
  }

  long size = ftell(f);
  if (size < 0) {
    fprintf(stderr, "sol: could not determine size of '%s'\n", path);
    fclose(f);
    return NULL;
  }
  rewind(f);

  char *buf = malloc((size_t)size + 1);
  if (buf == NULL) {
    fprintf(stderr, "sol: out of memory reading '%s'\n", path);
    fclose(f);
    return NULL;
  }

  size_t read = fread(buf, 1, (size_t)size, f);
  fclose(f);

  if (read != (size_t)size) {
    fprintf(stderr, "sol: failed to read entire file '%s'\n", path);
    free(buf);
    return NULL;
  }

  buf[size] = '\0';
  if (out_size != NULL) {
    *out_size = size;
  }
  return buf;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file.sun>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *path = argv[1];

  if (!has_sun_extension(path)) {
    fprintf(stderr, "sol: '%s' is not a .sun file\n", path);
    return EXIT_FAILURE;
  }

  char *source = read_file(path, NULL);
  if (source == NULL) {
    return EXIT_FAILURE;
  }

  lexer_t *lexer = new_lexer();
  lex(lexer, source);

  parser_t *parser = new_parser(lexer);
  parse(parser);
  print_ast(parser->ast);
  printf("\n");

  generator_t *gen = new_gen(parser);
  generate(gen);
  disassemble(gen, NULL);

  free_gen(gen);
  free_parser(parser);
  free_lexer(lexer);
  free(source);

  return EXIT_SUCCESS;
}
