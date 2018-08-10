#include "lbuiltin.h"

/* C files included here. This file was split for clarity only. */
#include "lbuiltin_condition.inc.c"
#include "lbuiltin_typed_operator.inc.c"

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Guards: instanciation. */
static const struct lguard guard_x_is_positive = {
    .condition= cnd_x_is_positive,
    .error= LERR_BAD_OPERAND
};
static const struct lguard guard_x_is_ul = {
    .condition= cnd_x_is_ul,
    .error= LERR_BAD_OPERAND
};
static const struct lguard guard_y_is_ul = {
    .condition= cnd_y_is_ul,
    .error= LERR_BAD_OPERAND
};
static const struct lguard guard_div_by_zero = {
    .condition= cnd_y_is_not_zero,
    .error= LERR_DIV_ZERO
};
static const struct lguard guard_x_is_integral = {
    .condition= cnd_x_is_integral,
    .error= LERR_BAD_OPERAND
};
static const struct lguard guard_are_integral = {
    .condition= cnd_are_integral,
    .error= LERR_BAD_OPERAND
};
static const struct lguard guard_are_numeric = {
    .condition= cnd_are_numeric,
    .error= LERR_BAD_OPERAND
};

/* Operator: declaration */
static const struct lguard* guards_op_add[] = {
    &guard_are_numeric,
};
const struct lsym lbuiltin_op_add = {
    .symbol       = "+",
    .guards       = guards_op_add,
    .guardc       = LENGTH(guards_op_add),
    .neutral      = &lzero,
    .op_num       = op_num_add,
    .cnd_overflow = cnd_num_add_overflow,
    .op_bignum    = mpz_add,
    .op_dbl       = op_dbl_add
};

static const struct lguard* guards_op_sub[] = {
    &guard_are_numeric,
};
const struct lsym lbuiltin_op_sub = {
    .symbol       = "-",
    .guards       = guards_op_sub,
    .guardc       = LENGTH(guards_op_sub),
    .neutral      = &lzero,
    .op_num       = op_num_sub,
    .cnd_overflow = cnd_num_sub_overflow,
    .op_bignum    = mpz_sub,
    .op_dbl       = op_dbl_sub
};
const struct lsym lbuiltin_op_sub_unary = {
    .symbol       = "-",
    .unary        = true,
    .guards       = guards_op_sub,
    .guardc       = LENGTH(guards_op_sub),
    .neutral      = &lzero,
    .op_num       = op_num_sub_unary,
    .cnd_overflow = cnd_num_sub_overflow,
    .op_bignum    = op_bignum_sub_unary,
    .op_dbl       = op_dbl_sub_unary
};

static const struct lguard* guards_op_mul[] = {
    &guard_are_numeric,
};
const struct lsym lbuiltin_op_mul = {
    .symbol       = "*",
    .guards       = guards_op_mul,
    .guardc       = LENGTH(guards_op_mul),
    .neutral      = &lone,
    .op_num       = op_num_mul,
    .cnd_overflow = cnd_num_mul_overflow,
    .op_bignum    = mpz_mul,
    .op_dbl       = op_dbl_mul
};

static const struct lguard* guards_op_div[] = {
    &guard_are_numeric,
    &guard_div_by_zero,
};
const struct lsym lbuiltin_op_div = {
    .symbol       = "/",
    .guards       = guards_op_div,
    .guardc       = LENGTH(guards_op_div),
    .neutral      = &lone,
    .op_num       = op_num_div,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_fdiv_q,
    .op_dbl       = op_dbl_div,
};

static const struct lguard* guards_op_mod[] = {
    &guard_are_numeric,
    &guard_div_by_zero,
    &guard_are_integral,
};
const struct lsym lbuiltin_op_mod = {
    .symbol       = "%",
    .guards       = guards_op_mod,
    .guardc       = LENGTH(guards_op_mod),
    .neutral      = &lone,
    .op_num       = op_num_mod,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_mod,
    .op_dbl       = op_dbl_nop
};

static const struct lguard* guards_op_fac[] = {
    &guard_x_is_integral,
    &guard_x_is_positive,
    &guard_x_is_ul
};
const struct lsym lbuiltin_op_fac = {
    .symbol       = "!",
    .unary        = true,
    .guards       = guards_op_fac,
    .guardc       = LENGTH(guards_op_fac),
    .neutral      = &lone,
    .op_num       = op_num_fact,
    .cnd_overflow = cnd_num_fact_overflow,
    .op_bignum    = op_bignum_fac,
    .op_dbl       = op_dbl_nop
};

static const struct lguard *guards_op_pow[] = {
    &guard_are_numeric,
    &guard_y_is_ul
};
const struct lsym lbuiltin_op_pow = {
    .symbol       = "^",
    .guards       = guards_op_pow,
    .guardc       = LENGTH(guards_op_pow),
    .neutral      = &lone,
    .op_num       = op_num_pow,
    .cnd_overflow = cnd_num_pow_overflow,
    .op_bignum    = op_bignum_pow,
    .op_dbl       = op_dbl_pow
};
