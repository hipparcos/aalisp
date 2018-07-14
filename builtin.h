#ifndef _H_BUILTIN_
#define _H_BUILTIN_

#include <stdbool.h>

#include "symbol.h"

extern const struct lsym builtin_op_add;
extern const struct lsym builtin_op_sub;
extern const struct lsym builtin_op_mul;
extern const struct lsym builtin_op_div;
extern const struct lsym builtin_op_mod;
extern const struct lsym builtin_op_fac;
extern const struct lsym builtin_op_pow;

#endif
