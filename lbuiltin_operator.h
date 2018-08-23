#ifndef _H_BUILTIN_OPERATOR_
#define _H_BUILTIN_OPERATOR_

#include "lval.h"
#include "lenv.h"

/** lbi_op_add is the + operator. */
int lbi_op_add(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_sub is the - operator. */
int lbi_op_sub(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_mul is the * operator. */
int lbi_op_mul(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_div is the / operator. */
int lbi_op_div(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_mod is the % operator. */
int lbi_op_mod(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_fac is the ! operator. */
int lbi_op_fac(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_pow is the ^ operator. */
int lbi_op_pow(struct lenv* env, const struct lval* arg, struct lval* acc);

/** lbi_op_eq is the == operator. */
int lbi_op_eq(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_neq is the != operator. */
int lbi_op_neq(struct lenv* env, const struct lval* arg, struct lval* acc);

/** lbi_op_gt is the > operator. */
int lbi_op_gt(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_gte is the >= operator. */
int lbi_op_gte(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_lt is the < operator. */
int lbi_op_lt(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_lte is the <= operator. */
int lbi_op_lte(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_and is the and operator. */
int lbi_op_and(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_or is the or operator. */
int lbi_op_or(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_not is the not operator. */
int lbi_op_not(struct lenv* env, const struct lval* arg, struct lval* acc);

#endif
