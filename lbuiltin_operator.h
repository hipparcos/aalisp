#ifndef _H_BUILTIN_OPERATOR_
#define _H_BUILTIN_OPERATOR_

#include "lval.h"
#include "lenv.h"

/** lbi_op_add is the + oerator. */
int lbi_op_add(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_sub is the - oerator. */
int lbi_op_sub(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_mul is the * oerator. */
int lbi_op_mul(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_div is the / oerator. */
int lbi_op_div(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_mod is the % oerator. */
int lbi_op_mod(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_fac is the ! oerator. */
int lbi_op_fac(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_pow is the ^ oerator. */
int lbi_op_pow(struct lenv* env, const struct lval* arg, struct lval* acc);

/** lbi_op_eq is the == oerator. */
int lbi_op_eq(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_neq is the != oerator. */
int lbi_op_neq(struct lenv* env, const struct lval* arg, struct lval* acc);

/** lbi_op_gt is the > oerator. */
int lbi_op_gt(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_gte is the >= oerator. */
int lbi_op_gte(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_lt is the < oerator. */
int lbi_op_lt(struct lenv* env, const struct lval* arg, struct lval* acc);
/** lbi_op_lte is the <= oerator. */
int lbi_op_lte(struct lenv* env, const struct lval* arg, struct lval* acc);

#endif
