#ifndef _INCLUDE_GEN_H_
#define _INCLUDE_GEN_H_

#include "common/arena.h"
#include "common/hashmap.h"
#include "common/types.h"
#include "common/vector.h"
#include "sparser.h"
#include <assert.h>

typedef enum {
  OP_INVALID,
  OP_HALT,

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_AND,
  OP_BITAND,
  OP_OR,
  OP_BITOR,
  OP_NOT,
  OP_BITNOT,
  OP_GREATER,
  OP_EQ_GREATER,
  OP_SMALLER,
  OP_EQ_SMALLER,
  OP_EQUAL,
  OP_NEQUAL,

  OP_PUSH_CONST,
  OP_DEF_LOCAL,
  OP_LOAD_LOCAL,
  OP_STORE_LOCAL,

  /* NOTE: Pops step, limit, i (in that order, top of stack first).
   * Pushes: step >= 0 ? (i <= limit) : (i >= limit)
   * This means VM will need to:
   * 1. Pop 3 values.
   * 2. Branch on step's sign (if step is non-negative i <= limit, negative i >=
   * limit).
   * 3. Push a single boolean.
   */
  OP_FOR_CHECK,

  OP_JMP,
  OP_JMP_TRUE,
  OP_JMP_FALSE,

  OP_COUNT, // always last
} opcode_t;

static_assert(OP_COUNT <= 256, "opcode_t exceeded 255");

typedef enum {
  CONST_DIGIT,
  CONST_STRING,
  CONST_BOOL,
  CONST_NIL,
} const_type_t;

typedef struct {
  const_type_t type;
  union {
    double digit;
    const char *str;
    bool bool;
  };
} const_t;

typedef struct {
  ushort slot;
} symbol;

typedef struct {
  ushort next_slot; // next free slot in this function's frame
  ushort max_slot;  // high-water mark, becomes the frame size
} func_frame_t;

typedef struct {
  parser_t *parser;
  arena_t *arena;

  ulong i;

  vector_t *const_table;

  ulong scope_depth;
  ulong nscopes;
  hashmap_t **scopes;

  func_frame_t *frames;
  ulong frame_depth;
  ulong nframes;

  ushort main_frame_size;

  ushort *code;
  ulong code_capacity;
} generator_t;

// creates a code generator
generator_t *new_gen(parser_t *parser);
// generates code, appended into generator->code
void generate(generator_t *generator);
// frees a generator
void free_gen(generator_t *generator);
// pretty print the generator's code
void disassemble(generator_t *gen, const char *name);

// encode an instruction into 2-bytes, where:
// -> bits 1-8 are the opcode
// -> bits 9-16 are the operand count
ushort encode(opcode_t opcode, uchar opcount);
// get the opcode of an instruction
opcode_t get_opcode(ushort instr);
// get the operand count of an instruction
uchar get_opcount(ushort instr);
// decode an instruction and store the information into opcode
// and opcount, both of which can be NULL
void decode(ushort instr, opcode_t *opcode, uchar *opcount);
#endif // !_INCLUDE_GEN_H_
