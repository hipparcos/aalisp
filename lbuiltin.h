#ifndef _H_BUILTIN_
#define _H_BUILTIN_

#include <stdbool.h>

#include "lsym.h"

extern const struct lsym lbuiltin_op_add;
extern const struct lsym lbuiltin_op_sub;
extern const struct lsym lbuiltin_op_mul;
extern const struct lsym lbuiltin_op_div;
extern const struct lsym lbuiltin_op_mod;
extern const struct lsym lbuiltin_op_fac;
extern const struct lsym lbuiltin_op_pow;

extern const struct lsym lbuiltin_head;
extern const struct lsym lbuiltin_tail;
extern const struct lsym lbuiltin_init;
extern const struct lsym lbuiltin_cons;
extern const struct lsym lbuiltin_len;
extern const struct lsym lbuiltin_join;
extern const struct lsym lbuiltin_list;
extern const struct lsym lbuiltin_eval;

#endif
