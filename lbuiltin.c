#include "lbuiltin.h"

#include "lfunc.h"
#include "lbuiltin_condition.h"
#include "lbuiltin_func.h"
#include "lbuiltin_operator.h"

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Operator: declaration */
static const struct lguard guards_op_add[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_op_add = {
    .symbol       = "+",
    .accumulator  = true,
    .neutral      = &lzero,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_add[0],
    .guardc       = LENGTH(guards_op_add),
    .op_num       = lbi_op_num_add,
    .cnd_overflow = lbi_cond_num_add_overflow,
    .op_bignum    = mpz_add,
    .op_dbl       = lbi_op_dbl_add
};

static const struct lguard guards_op_sub[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_op_sub = {
    .symbol       = "-",
    .accumulator  = true,
    .neutral      = &lzero,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_sub[0],
    .guardc       = LENGTH(guards_op_sub),
    .op_num       = lbi_op_num_sub,
    .cnd_overflow = lbi_cond_num_sub_overflow,
    .op_bignum    = mpz_sub,
    .op_dbl       = lbi_op_dbl_sub
};

static const struct lguard guards_op_mul[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_op_mul = {
    .symbol       = "*",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_mul[0],
    .guardc       = LENGTH(guards_op_mul),
    .op_num       = lbi_op_num_mul,
    .cnd_overflow = lbi_cond_num_mul_overflow,
    .op_bignum    = mpz_mul,
    .op_dbl       = lbi_op_dbl_mul
};

static const struct lguard guards_op_div[] = {
    {.condition= lbi_cond_is_numeric,  .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_not_zero, .error= LERR_DIV_ZERO},
};
const struct lfunc lbuiltin_op_div = {
    .symbol       = "/",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_div[0],
    .guardc       = LENGTH(guards_op_div),
    .op_num       = lbi_op_num_div,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_fdiv_q,
    .op_dbl       = lbi_op_dbl_div,
};

static const struct lguard guards_op_mod[] = {
    {.condition= lbi_cond_is_integral, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_not_zero, .error= LERR_DIV_ZERO},
};
const struct lfunc lbuiltin_op_mod = {
    .symbol       = "%",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_mod[0],
    .guardc       = LENGTH(guards_op_mod),
    .op_num       = lbi_op_num_mod,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_mod,
    .op_dbl       = lbi_op_dbl_nop
};

static const struct lguard guards_op_fac[] = {
    {.condition= lbi_cond_is_integral, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_positive, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_x_is_ul,     .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_op_fac = {
    .symbol       = "!",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_op_fac[0],
    .guardc       = LENGTH(guards_op_fac),
    .op_num       = lbi_op_num_fac,
    .cnd_overflow = lbi_cond_num_fac_overflow,
    .op_bignum    = lbi_op_bignum_fac,
    .op_dbl       = lbi_op_dbl_nop
};

static const struct lguard guards_op_pow[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_x_is_ul,    .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_op_pow = {
    .symbol       = "^",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_pow[0],
    .guardc       = LENGTH(guards_op_pow),
    .op_num       = lbi_op_num_pow,
    .cnd_overflow = lbi_cond_num_pow_overflow,
    .op_bignum    = lbi_op_bignum_pow,
    .op_dbl       = lbi_op_dbl_pow
};

static const struct lguard guards_head[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_head = {
    .symbol       = "head",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_head[0],
    .guardc       = LENGTH(guards_head),
    .func         = lbi_func_head,
};

static const struct lguard guards_tail[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_tail = {
    .symbol       = "tail",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_tail[0],
    .guardc       = LENGTH(guards_tail),
    .func         = lbi_func_tail,
};

static const struct lguard guards_init[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_init = {
    .symbol       = "init",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_init[0],
    .guardc       = LENGTH(guards_init),
    .func         = lbi_func_init,
};

static const struct lguard guards_cons[] = {
    {.condition= lbi_cond_qexpr, .argn= 2, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_cons = {
    .symbol       = "cons",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_cons[0],
    .guardc       = LENGTH(guards_cons),
    .func         = lbi_func_cons,
};

static const struct lguard guards_len[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_len = {
    .symbol       = "len",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_len[0],
    .guardc       = LENGTH(guards_len),
    .func         = lbi_func_len,
};

static const struct lguard guards_join[] = {
    {.condition= lbi_cond_qexpr, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_join = {
    .symbol       = "join",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_join[0],
    .guardc       = LENGTH(guards_join),
    .func         = lbi_func_join,
};

static const struct lguard guards_list[] = {
};
const struct lfunc lbuiltin_list = {
    .symbol       = "list",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_list[0],
    .guardc       = LENGTH(guards_list),
    .func         = lbi_func_list,
};

static const struct lguard guards_eval[] = {
};
const struct lfunc lbuiltin_eval = {
    .symbol       = "eval",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_eval[0],
    .guardc       = LENGTH(guards_eval),
    .func         = lbi_func_eval,
};

static const struct lguard guards_def[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list_of_sym, .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct lfunc lbuiltin_def = {
    .symbol       = "def",
    .min_argc     =  2,
    .max_argc     =  -1,
    .guards       = &guards_def[0],
    .guardc       = LENGTH(guards_def),
    .func         = lbi_func_def,
};
