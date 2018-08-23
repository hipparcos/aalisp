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

/* Boolean operators. */
extern const struct lfunc lbuiltin_op_eq;
extern const struct lfunc lbuiltin_op_neq;
extern const struct lfunc lbuiltin_op_gt;
extern const struct lfunc lbuiltin_op_gte;
extern const struct lfunc lbuiltin_op_lt;
extern const struct lfunc lbuiltin_op_lte;
extern const struct lfunc lbuiltin_op_and;
extern const struct lfunc lbuiltin_op_or;
extern const struct lfunc lbuiltin_op_not;

/* Control flow functions. */
extern const struct lfunc lbuiltin_if;
extern const struct lfunc lbuiltin_loop;

/* List manipulation functions. */
extern const struct lfunc lbuiltin_head;
extern const struct lfunc lbuiltin_tail;
extern const struct lfunc lbuiltin_init;
extern const struct lfunc lbuiltin_last;
extern const struct lfunc lbuiltin_cons;
extern const struct lfunc lbuiltin_len;
extern const struct lfunc lbuiltin_join;
extern const struct lfunc lbuiltin_list;
extern const struct lfunc lbuiltin_eval;
extern const struct lfunc lbuiltin_map;

/* Environment manipulation functions. */
extern const struct lfunc lbuiltin_def;
extern const struct lfunc lbuiltin_override;
extern const struct lfunc lbuiltin_put;
extern const struct lfunc lbuiltin_fun;
extern const struct lfunc lbuiltin_lambda;
extern const struct lfunc lbuiltin_pack;
extern const struct lfunc lbuiltin_unpack;

/* IO functions. */
extern const struct lfunc lbuiltin_print;

#endif
