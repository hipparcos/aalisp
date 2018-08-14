#include "lbuiltin.h"

/* C files included here. This file was split for clarity only. */
#include "lbuiltin_condition.inc.c"
#include "lbuiltin_typed_operator.inc.c"
#include "lbuiltin_func.h"

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Operator: declaration */
static const struct lguard guards_op_add[] = {
    {cnd_are_numeric,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_op_add = {
    .symbol       = "+",
    .guards       = &guards_op_add[0],
    .guardc       = LENGTH(guards_op_add),
    .neutral      = &lzero,
    .op_num       = op_num_add,
    .cnd_overflow = cnd_num_add_overflow,
    .op_bignum    = mpz_add,
    .op_dbl       = op_dbl_add
};

static const struct lguard guards_op_sub[] = {
    {cnd_are_numeric,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_op_sub = {
    .symbol       = "-",
    .guards       = &guards_op_sub[0],
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
    .guards       = &guards_op_sub[0],
    .guardc       = LENGTH(guards_op_sub),
    .neutral      = &lzero,
    .op_num       = op_num_sub_unary,
    .cnd_overflow = cnd_num_sub_overflow,
    .op_bignum    = op_bignum_sub_unary,
    .op_dbl       = op_dbl_sub_unary
};

static const struct lguard guards_op_mul[] = {
    {cnd_are_numeric,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_op_mul = {
    .symbol       = "*",
    .guards       = &guards_op_mul[0],
    .guardc       = LENGTH(guards_op_mul),
    .neutral      = &lone,
    .op_num       = op_num_mul,
    .cnd_overflow = cnd_num_mul_overflow,
    .op_bignum    = mpz_mul,
    .op_dbl       = op_dbl_mul
};

static const struct lguard guards_op_div[] = {
    {cnd_are_numeric,  LERR_BAD_OPERAND},
    {cnd_y_is_not_zero, LERR_DIV_ZERO},
};
const struct lsym lbuiltin_op_div = {
    .symbol       = "/",
    .guards       = &guards_op_div[0],
    .guardc       = LENGTH(guards_op_div),
    .neutral      = &lone,
    .op_num       = op_num_div,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_fdiv_q,
    .op_dbl       = op_dbl_div,
};

static const struct lguard guards_op_mod[] = {
    {cnd_are_integral,  LERR_BAD_OPERAND},
    {cnd_y_is_not_zero, LERR_DIV_ZERO},
};
const struct lsym lbuiltin_op_mod = {
    .symbol       = "%",
    .guards       = &guards_op_mod[0],
    .guardc       = LENGTH(guards_op_mod),
    .neutral      = &lone,
    .op_num       = op_num_mod,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_mod,
    .op_dbl       = op_dbl_nop
};

static const struct lguard guards_op_fac[] = {
    {cnd_x_is_integral, LERR_BAD_OPERAND},
    {cnd_x_is_positive, LERR_BAD_OPERAND},
    {cnd_x_is_ul,       LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_op_fac = {
    .symbol       = "!",
    .unary        = true,
    .guards       = &guards_op_fac[0],
    .guardc       = LENGTH(guards_op_fac),
    .neutral      = &lone,
    .op_num       = op_num_fact,
    .cnd_overflow = cnd_num_fact_overflow,
    .op_bignum    = op_bignum_fac,
    .op_dbl       = op_dbl_nop
};

static const struct lguard guards_op_pow[] = {
    {cnd_are_numeric, LERR_BAD_OPERAND},
    {cnd_x_is_ul,     LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_op_pow = {
    .symbol       = "^",
    .guards       = &guards_op_pow[0],
    .guardc       = LENGTH(guards_op_pow),
    .neutral      = &lone,
    .op_num       = op_num_pow,
    .cnd_overflow = cnd_num_pow_overflow,
    .op_bignum    = op_bignum_pow,
    .op_dbl       = op_dbl_pow
};

static const struct lguard guards_head[] = {
    {lbi_cond_qexpr, LERR_BAD_OPERAND},
    {lbi_cond_list,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_head = {
    .symbol       = "head",
    .guards       = &guards_head[0],
    .guardc       = LENGTH(guards_head),
    .neutral      = &lnil,
    .op_all       = lbi_func_head,
};

static const struct lguard guards_tail[] = {
    {lbi_cond_qexpr, LERR_BAD_OPERAND},
    {lbi_cond_list,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_tail = {
    .symbol       = "tail",
    .guards       = &guards_tail[0],
    .guardc       = LENGTH(guards_tail),
    .neutral      = &lnil,
    .op_all       = lbi_func_tail,
};

static const struct lguard guards_init[] = {
    {lbi_cond_qexpr, LERR_BAD_OPERAND},
    {lbi_cond_list,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_init = {
    .symbol       = "init",
    .guards       = &guards_init[0],
    .guardc       = LENGTH(guards_init),
    .neutral      = &lnil,
    .op_all       = lbi_func_init,
};

static const struct lguard guards_join[] = {
    {lbi_cond_qexpr, LERR_BAD_OPERAND},
    {lbi_cond_list,  LERR_BAD_OPERAND},
};
const struct lsym lbuiltin_join = {
    .symbol       = "join",
    .guards       = &guards_join[0],
    .guardc       = LENGTH(guards_join),
    .neutral      = &lnil,
    .op_all       = lbi_func_join,
};

static const struct lguard guards_list[] = {
};
const struct lsym lbuiltin_list = {
    .symbol       = "list",
    .guards       = &guards_list[0],
    .guardc       = LENGTH(guards_list),
    .neutral      = &lnil,
    .op_all       = lbi_func_list,
};

static const struct lguard guards_eval[] = {
};
const struct lsym lbuiltin_eval = {
    .symbol       = "eval",
    .guards       = &guards_eval[0],
    .guardc       = LENGTH(guards_eval),
    .neutral      = &lnil,
    .op_all       = lbi_func_eval,
};
