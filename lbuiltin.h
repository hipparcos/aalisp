#ifndef _H_BUILTIN_
#define _H_BUILTIN_

#include <stdbool.h>

#include "lval.h"
#include "lenv.h"

/* Arithmetic operators. */
int lbuiltin_op_add(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_sub(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_mul(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_div(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_mod(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_pow(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_op_fac(struct lenv*, const struct lval*, struct lval*);

/* List functions. */
int lbuiltin_head(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_tail(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_init(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_cons(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_len(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_join(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_list(struct lenv*, const struct lval*, struct lval*);
int lbuiltin_eval(struct lenv*, const struct lval*, struct lval*);

#endif
