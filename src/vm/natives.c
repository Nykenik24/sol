#include "sol/vm/vm.h"
#include "vmhelpers.h"
#include "vmnatives.h"
#include <stdio.h>

Value native_print(VM *vm, Value *args, size arg_n) {
  for (size i = 0; i < arg_n; i++) {
    Value v = args[i];

    printf("%s", sol_val_to_str(v));

    if (i + 1 < arg_n)
      printf("\t");
  }

  printf("\n");

  return make_nil();
}
