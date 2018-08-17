#ifndef _H_BUILTIN_
#define _H_BUILTIN_

#include <stdbool.h>

#include "lfunc.h"

/* Arithmetic operators. */
extern const struct lfunc lbuiltin_op_add;
extern const struct lfunc lbuiltin_op_sub;
extern const struct lfunc lbuiltin_op_mul;
extern const struct lfunc lbuiltin_op_div;
extern const struct lfunc lbuiltin_op_mod;
extern const struct lfunc lbuiltin_op_pow;
extern const struct lfunc lbuiltin_op_fac;

/* List manipulation functions. */
extern const struct lfunc lbuiltin_head;
extern const struct lfunc lbuiltin_tail;
extern const struct lfunc lbuiltin_init;
extern const struct lfunc lbuiltin_cons;
extern const struct lfunc lbuiltin_len;
extern const struct lfunc lbuiltin_join;
extern const struct lfunc lbuiltin_list;
extern const struct lfunc lbuiltin_eval;

/* Environment manipulation functions. */
extern const struct lfunc lbuiltin_def;
extern const struct lfunc lbuiltin_put;
extern const struct lfunc lbuiltin_lambda;

#endif
