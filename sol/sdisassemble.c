#include "common/types.h"
#include "common/vector.h"
#include "sgen.h"

#include <stdio.h>

static const char *opcode_name(opcode_t op) {
  switch (op) {
  case OP_INVALID:
    return "INVALID";
  case OP_HALT:
    return "HALT";
  case OP_ADD:
    return "ADD";
  case OP_SUB:
    return "SUB";
  case OP_MUL:
    return "MUL";
  case OP_DIV:
    return "DIV";
  case OP_MOD:
    return "MOD";
  case OP_AND:
    return "AND";
  case OP_BITAND:
    return "BIT AND";
  case OP_OR:
    return "OR";
  case OP_BITOR:
    return "BIT OR";
  case OP_NOT:
    return "NOT";
  case OP_BITNOT:
    return "BITNOT";
  case OP_GREATER:
    return "GREATER";
  case OP_EQ_GREATER:
    return "EQUAL/GREATER";
  case OP_SMALLER:
    return "SMALLER";
  case OP_EQ_SMALLER:
    return "EQUAL/SMALLER";
  case OP_EQUAL:
    return "EQUAL";
  case OP_NEQUAL:
    return "NOT EQUAL";
  case OP_PUSH_CONST:
    return "PUSH CONST";
  case OP_DEF_LOCAL:
    return "DEF LOCAL";
  case OP_LOAD_LOCAL:
    return "LOAD LOCAL";
  case OP_STORE_LOCAL:
    return "STORE LOCAL";
  case OP_JMP:
    return "JUMP";
  case OP_JMP_TRUE:
    return "JUMP (TRUE)";
  case OP_JMP_FALSE:
    return "JUMP (FALSE)";
  case OP_FOR_CHECK:
    return "FOR CHECK";
  case OP_COUNT:
    return "<COUNT>";
  }
  return "<UNKNOWN>";
}

static void print_const(const const_t *c) {
  switch (c->type) {
  case CONST_DIGIT:
    printf("%g", c->digit);
    return;
  case CONST_STRING:
    printf("\"%s\"", c->str);
    return;
  case CONST_BOOL:
    printf("%s", c->bool ? "true" : "false");
    return;
  case CONST_NIL:
    printf("nil");
    return;
  }
  printf("<unknown const>");
}

ulong disassemble_instr(generator_t *gen, ulong offset) {
  ushort instr = gen->code[offset];
  opcode_t op;
  uchar opcount;
  decode(instr, &op, &opcount);

  printf("%6lu  %-12s", offset, opcode_name(op));

  for (uchar i = 0; i < opcount; i++) {
    ushort operand = gen->code[offset + 1 + i];
    printf(" %5hu", operand);
  }

  if (op == OP_PUSH_CONST && opcount >= 1) {
    ushort const_idx = gen->code[offset + 1];
    printf("  ; ");
    if (const_idx < vector_size(gen->const_table)) {
      const const_t *c = vector_get(gen->const_table, const_idx);
      print_const(c);
    } else {
      printf("<const index %hu out of range>", const_idx);
    }
  } else if (op == OP_DEF_LOCAL && opcount >= 1) {
    printf("  ; slot %hu", gen->code[offset + 1]);
  }

  printf("\n");

  return offset + 1 + opcount;
}

void disassemble(generator_t *gen, const char *name) {
  printf("== %s ==\n", name ? name : "code");
  ulong offset = 0;
  while (offset < gen->i) {
    offset = disassemble_instr(gen, offset);
  }
}
