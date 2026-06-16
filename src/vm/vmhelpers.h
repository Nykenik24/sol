#include "sol/types.h"
#include "sol/vm/vm.h"

Value make_nil();
Value make_number(double n);
Value make_string(const char *s);
Value make_bool(bool b);
Value make_native(Fn fn);

Env *env_create();
void env_destroy(Env *e);
void env_set(Env *e, const char *name, Value v);
Value env_get(Env *e, const char *name);
