#include "sgen.h"
#include "common/arena.h"
#include "common/error.h"
#include "common/types.h"
#include "common/vector.h"
#include "slexer.h"
#include "snode.h"
#include "sparser.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define GEN_CODE_INITIAL_CAPACITY 256

generator_t *new_gen(parser_t *parser) {
  generator_t *gen = calloc(1, sizeof(generator_t));
  if (gen == NULL) {
    return NULL;
  }

  gen->parser = parser;
  gen->arena = new_arena();
  gen->const_table = new_vector(sizeof(const_t));

  gen->code = malloc(GEN_CODE_INITIAL_CAPACITY * sizeof(ushort));
  gen->code_capacity = GEN_CODE_INITIAL_CAPACITY;
  gen->i = 0;

  return gen;
}

void free_gen(generator_t *gen) {
  if (gen == NULL) {
    return;
  }

  for (ulong i = 0; i < gen->scope_depth; i++) {
    free_map(gen->scopes[i]);
  }
  free(gen->scopes);
  free(gen->frames);

  free_vector(gen->const_table);
  free_arena(gen->arena);

  free(gen->code);
  free(gen);
}

ushort encode(opcode_t opcode, uchar opcount) {
  assert(opcode >= 0 && opcode <= 0xFF);
  return (ushort)(((ushort)opcode << 8) | opcount);
}

opcode_t get_opcode(ushort instr) { return (opcode_t)((instr >> 8) & 0xFF); }

uchar get_opcount(ushort instr) { return (uchar)(instr & 0xFF); }

void decode(ushort instr, opcode_t *opcode, uchar *opcount) {
  if (opcode)
    *opcode = get_opcode(instr);
  if (opcount)
    *opcount = get_opcount(instr);
}

static void code_ensure_capacity(generator_t *gen, ulong needed) {
  if (gen->i + needed <= gen->code_capacity) {
    return;
  }
  ulong new_capacity = gen->code_capacity * 2;
  while (new_capacity < gen->i + needed) {
    new_capacity *= 2;
  }
  ushort *new_code = realloc(gen->code, new_capacity * sizeof(ushort));
  if (new_code == NULL) {
    return;
  }
  gen->code = new_code;
  gen->code_capacity = new_capacity;
}

static const_t make_digit(double n) {
  return (const_t){.type = CONST_DIGIT, .digit = n};
}

static const_t make_str(const char *str) {
  return (const_t){.type = CONST_STRING, .str = str};
}

static const_t make_bool(bool b) {
  return (const_t){.type = CONST_BOOL, .bool = b};
}

static const_t make_nil() { return (const_t){.type = CONST_NIL}; }

static ulong push_const(generator_t *gen, const_t c) {
  for (ulong i = 0; i < vector_size(gen->const_table); i++) {
    const_t c2 = *(const_t *)vector_get(gen->const_table, i);
    if (c.type == c2.type) {
      switch (c.type) {
      case CONST_DIGIT:
        if (c.digit == c2.digit)
          return i;
        break;
      case CONST_STRING:
        if (strcmp(c.str, c2.str) == 0)
          return i;
        break;
      case CONST_BOOL:
        if (c.bool == c2.bool)
          return i;
        break;
      case CONST_NIL:
        return i;
      }
    }
  }

  ulong idx = vector_size(gen->const_table);
  push_back(gen->const_table, &c);
  return idx;
}

#define SCOPES_INITIAL_CAPACITY 8

static void scopes_grow(generator_t *gen) {
  ulong new_capacity =
      gen->nscopes == 0 ? SCOPES_INITIAL_CAPACITY : gen->nscopes * 2;
  hashmap_t **new_scopes =
      realloc(gen->scopes, new_capacity * sizeof(hashmap_t *));
  if (new_scopes == NULL) {
    return;
  }
  gen->scopes = new_scopes;
  gen->nscopes = new_capacity;
}

void scope_push(generator_t *gen) {
  if (gen->scope_depth == gen->nscopes) {
    scopes_grow(gen);
  }
  gen->scopes[gen->scope_depth] = new_map();
  gen->scope_depth++;
}

void scope_pop(generator_t *gen) {
  if (gen->scope_depth < 1) {
    return;
  }
  gen->scope_depth--;
  free_map(gen->scopes[gen->scope_depth]);
  gen->scopes[gen->scope_depth] = NULL;
}

void scope_define(generator_t *gen, const char *name, symbol *sym) {
  if (gen->scope_depth == 0) {
    return;
  }
  hashmap_t *current = gen->scopes[gen->scope_depth - 1];
  map_push(current, name, sym);
}

symbol *scope_resolve_local(generator_t *gen, const char *name) {
  if (gen->scope_depth == 0) {
    return NULL;
  }
  hashmap_t *current = gen->scopes[gen->scope_depth - 1];
  return (symbol *)map_get(current, name);
}

symbol *scope_resolve(generator_t *gen, const char *name) {
  for (ulong i = gen->scope_depth; i > 0; i--) {
    hashmap_t *scope = gen->scopes[i - 1];
    symbol *sym = (symbol *)map_get(scope, name);
    if (sym != NULL) {
      return sym;
    }
  }
  return NULL;
}

#define FRAMES_INITIAL_CAPACITY 8

static void frames_grow(generator_t *gen) {
  ulong new_capacity =
      gen->nframes == 0 ? FRAMES_INITIAL_CAPACITY : gen->nframes * 2;
  func_frame_t *new_frames =
      realloc(gen->frames, new_capacity * sizeof(func_frame_t));
  if (new_frames == NULL) {
    return;
  }
  gen->frames = new_frames;
  gen->nframes = new_capacity;
}

void frame_push(generator_t *gen) {
  if (gen->frame_depth == gen->nframes) {
    frames_grow(gen);
  }
  gen->frames[gen->frame_depth].next_slot = 0;
  gen->frames[gen->frame_depth].max_slot = 0;
  gen->frame_depth++;
}

// NOTE: returns the frame's final size (max_slot), so the caller can stash it
// wherever per-function frame-size metadata belongs (e.g. on a proto/
// closure object once those exist)
ushort frame_pop(generator_t *gen) {
  assert(gen->frame_depth > 0 && "frame_pop with no active frame");
  gen->frame_depth--;
  return gen->frames[gen->frame_depth].max_slot;
}

static ushort next_local_slot(generator_t *gen) {
  assert(gen->frame_depth > 0 && "no active function frame");
  func_frame_t *frame = &gen->frames[gen->frame_depth - 1];
  ushort slot = frame->next_slot++;
  if (frame->next_slot > frame->max_slot) {
    frame->max_slot = frame->next_slot;
  }
  return slot;
}

static ulong emit(generator_t *gen, opcode_t op, int op_count, ...) {
  va_list args;
  va_start(args, op_count);
  gen->code[gen->i++] = encode(op, op_count);
  ulong idx = gen->i;
  for (ulong i = 0; i < op_count; i++)
    gen->code[gen->i++] = va_arg(args, int);
  va_end(args);
  return idx;
}

static ulong emit_0_op(generator_t *gen, opcode_t op) {
  gen->code[gen->i++] = encode(op, 0);
  return gen->i;
}

static void g_lit(generator_t *gen, node_t *node) {
  switch (node->kind) {
  case SOL_NODE_INT:
  case SOL_NODE_FLOAT:
  case SOL_NODE_HEX_DIGIT: {
    ushort c = push_const(gen, make_digit(node->u.num));
    emit(gen, OP_PUSH_CONST, 1, c);
    break;
  }
  case SOL_NODE_STRING: {
    ushort c = push_const(gen, make_str(node->u.str));
    emit(gen, OP_PUSH_CONST, 1, c);
    break;
  }
  case SOL_NODE_TRUE:
  case SOL_NODE_FALSE: {
    ushort c =
        push_const(gen, make_bool(node->kind == SOL_NODE_TRUE ? true : false));
    emit(gen, OP_PUSH_CONST, 1, c);
    break;
  }
  case SOL_NODE_NIL: {
    ushort c = push_const(gen, make_nil());
    emit(gen, OP_PUSH_CONST, 1, c);
    break;
  }
  default:
    sol_fatal_internal("this isn't a literal!\n");
  }
}

static void g_exp(generator_t *gen, node_t *node);

static void g_unop(generator_t *gen, node_t *node) {
  token_t *op = node->u.unop.op;
  switch (op->type) {
  case SOL_TK_SYM_SUB:
    push_const(gen, make_digit(0));
    g_exp(gen, node->u.unop.operand);
    emit_0_op(gen, OP_SUB);
    break;
  case SOL_TK_KW_NOT:
    g_exp(gen, node->u.unop.operand);
    emit_0_op(gen, OP_NOT);
    break;
  case SOL_TK_SYM_BITNOT:
    g_exp(gen, node->u.unop.operand);
    emit_0_op(gen, OP_BITNOT);
    break;
  default:
    sol_fatal("'%s' isn't a supported unary operator\n", op->txt);
  }
}

static void g_binop(generator_t *gen, node_t *node) {
  token_t *op = node->u.binop.op;

  g_exp(gen, node->u.binop.left);
  g_exp(gen, node->u.binop.right);

  switch (op->type) {
  case SOL_TK_SYM_ADD:
    emit_0_op(gen, OP_ADD);
    return;
  case SOL_TK_SYM_SUB:
    emit_0_op(gen, OP_SUB);
    return;
  case SOL_TK_SYM_MUL:
    emit_0_op(gen, OP_MUL);
    return;
  case SOL_TK_SYM_DIV:
    emit_0_op(gen, OP_DIV);
    return;
  case SOL_TK_SYM_MOD:
    emit_0_op(gen, OP_MOD);
    return;

  case SOL_TK_KW_AND:
    emit_0_op(gen, OP_AND);
    return;
  case SOL_TK_KW_OR:
    emit_0_op(gen, OP_OR);
    return;
  case SOL_TK_SYM_BITAND:
    emit_0_op(gen, OP_BITAND);
    return;
  case SOL_TK_SYM_BITOR:
    emit_0_op(gen, OP_BITOR);
    return;

  default:
    sol_fatal("'%s' isn't a supported binary operator\n", op->txt);
  }
}

static void g_prefixexp(generator_t *gen, node_t *node) {}

static void g_infixexp(generator_t *gen, node_t *node) {

};

static void g_exp(generator_t *gen, node_t *node) {
  switch (node->kind) {
  case SOL_NODE_INT:
  case SOL_NODE_FLOAT:
  case SOL_NODE_HEX_DIGIT:
  case SOL_NODE_STRING:
  case SOL_NODE_TRUE:
  case SOL_NODE_FALSE:
  case SOL_NODE_NIL:
    g_lit(gen, node);
    break;

  case SOL_NODE_UNOP:
    g_unop(gen, node);
    break;

  case SOL_NODE_BINOP:
    g_binop(gen, node);
    break;

  case SOL_NODE_ASSIGN:
    g_infixexp(gen, node);
    break;

  case SOL_NODE_FUNC_CALL:
  case SOL_NODE_METHOD_CALL:
  case SOL_NODE_INDEX:
  case SOL_NODE_FIELD:
    g_prefixexp(gen, node);
    break;

  default:
    sol_fatal_internal("this isn't an expression!\n");
  }
}

static void g_stmt(generator_t *gen, node_t *node);

static void g_decl(generator_t *gen, node_t *node) {
  ulong n = node->u.decl.n;
  ulong value_n = node->u.decl.value_n;
  assert(value_n <= n && "parser should reject value_n > n before codegen");

  for (ulong i = 0; i < value_n; i++) {
    g_exp(gen, node->u.decl.values[i]);
  }

  for (ulong i = value_n; i < n; i++) {
    ulong nil = push_const(gen, make_nil());
    emit(gen, OP_PUSH_CONST, 1, nil);
  }

  for (ulong i = n; i-- > 0;) {
    const char *name = node->u.decl.names[i];

    // WARN: unused
    const char *attrib = node->u.decl.attribs[i];
    (void)attrib;

    symbol *sym = scope_resolve_local(gen, name);
    if (sym != NULL) {
      sol_fatal("redeclaration of variable '%s' in local scope\n", name);
    }

    sym = arena_alloc(gen->arena, sizeof(symbol));
    sym->slot = next_local_slot(gen);

    scope_define(gen, name, sym);
    emit(gen, OP_DEF_LOCAL, 1, sym->slot);
  }
}

static void g_block(generator_t *gen, node_t *node) {
  scope_push(gen);
  for (ulong i = 0; i < node->u.block.n; i++) {
    g_stmt(gen, node->u.block.stmts[i]);
  }
  scope_pop(gen);
}

static void g_stmt(generator_t *gen, node_t *node) {
  switch (node->kind) {
  case SOL_NODE_DECL:
    g_decl(gen, node);
    break;
  case SOL_NODE_BLOCK:
    g_block(gen, node);
    break;
  default:
    sol_fatal_internal("unsupported statement %d\n", node->kind);
  }
}

void generate(generator_t *gen) {
  frame_push(gen);
  scope_push(gen);

  node_t *root = gen->parser->ast;
  for (ulong i = 0; i < root->u.block.n; i++) {
    g_stmt(gen, root->u.block.stmts[i]);
  }

  scope_pop(gen);
  ushort mfs = frame_pop(gen);
  gen->main_frame_size = mfs;
}
