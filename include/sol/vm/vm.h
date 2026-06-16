#ifndef SOL_INCLUDE__VM_VM_H
#define SOL_INCLUDE__VM_VM_H

#include "sol/parser/node.h"
#include "sol/types.h"

typedef struct Value Value;
typedef struct VM VM;

typedef Value (*Fn)(VM *vm, Value *args, size arg_n);

typedef enum {
  VAL_NIL,
  VAL_NUMBER,
  VAL_STRING,
  VAL_BOOLEAN,
  VAL_NATIVE,
} ValueType;

struct Value {
  ValueType type;

  union {
    double num;
    const char *str;
    bool b;

    struct {
      Node *body;
    } func;

    struct {
      Fn fn;
    } native;
  } as;
};

// Value type to string.
char *sol_vtype_to_str(ValueType vt);

// Value as string
char *sol_val_to_str(Value v);

typedef struct Env {
  struct Env *parent;

  const char **names;
  Value *values;
  size count;
  size capacity;
} Env;

struct VM {
  Env *env;
};

// Create a VM.
VM *sol_vm_create(void);

// Destroy a VM.
void sol_vm_destroy(VM *vm);

// Execute an AST.
void sol_vm_exec(VM *vm, Node **nodes, size n);

#endif
