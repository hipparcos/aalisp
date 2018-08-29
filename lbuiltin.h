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
extern const struct lfunc lbuiltin_index;
extern const struct lfunc lbuiltin_elem;
extern const struct lfunc lbuiltin_take;
extern const struct lfunc lbuiltin_drop;
extern const struct lfunc lbuiltin_cons;
extern const struct lfunc lbuiltin_len;
extern const struct lfunc lbuiltin_join;
extern const struct lfunc lbuiltin_list;
extern const struct lfunc lbuiltin_seq;
extern const struct lfunc lbuiltin_eval;
extern const struct lfunc lbuiltin_map;
extern const struct lfunc lbuiltin_filter;
extern const struct lfunc lbuiltin_fold;
extern const struct lfunc lbuiltin_reverse;
extern const struct lfunc lbuiltin_all;
extern const struct lfunc lbuiltin_any;
extern const struct lfunc lbuiltin_zip;
extern const struct lfunc lbuiltin_sort;
extern const struct lfunc lbuiltin_mix;

/* Environment manipulation functions. */
extern const struct lfunc lbuiltin_def;
extern const struct lfunc lbuiltin_override;
extern const struct lfunc lbuiltin_put;
extern const struct lfunc lbuiltin_fun;
extern const struct lfunc lbuiltin_load;

/* Functions manipulation functions. */
extern const struct lfunc lbuiltin_lambda;
extern const struct lfunc lbuiltin_pack;
extern const struct lfunc lbuiltin_unpack;
extern const struct lfunc lbuiltin_partial;

/* IO functions. */
extern const struct lfunc lbuiltin_print;

/* Debug functions. */
extern const struct lfunc lbuiltin_debug_env;
extern const struct lfunc lbuiltin_debug_fun;
extern const struct lfunc lbuiltin_debug_val;

/* Error functions. */
extern const struct lfunc lbuiltin_error;

#endif
