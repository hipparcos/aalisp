#include "lbuiltin.h"

#include "lbuiltin_exec.h"
#include "lbuiltin_func.h"
#include "lbuiltin_operator.h"

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Operator: declaration */
static const struct lguard guards_op_add[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_op_add = {
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
int lbuiltin_op_add(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_add, env, args, acc);
}

static const struct lguard guards_op_sub[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_op_sub = {
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
int lbuiltin_op_sub(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_sub, env, args, acc);
}

static const struct lguard guards_op_mul[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_op_mul = {
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
int lbuiltin_op_mul(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_mul, env, args, acc);
}

static const struct lguard guards_op_div[] = {
    {.condition= lbi_cond_is_numeric,  .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_not_zero, .error= LERR_DIV_ZERO},
};
const struct ldescriptor lbi_descriptor_op_div = {
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
int lbuiltin_op_div(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_div, env, args, acc);
}

static const struct lguard guards_op_mod[] = {
    {.condition= lbi_cond_is_integral, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_not_zero, .error= LERR_DIV_ZERO},
};
const struct ldescriptor lbi_descriptor_op_mod = {
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
int lbuiltin_op_mod(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_mod, env, args, acc);
}

static const struct lguard guards_op_fac[] = {
    {.condition= lbi_cond_is_integral, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_is_positive, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_x_is_ul,     .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_op_fac = {
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
int lbuiltin_op_fac(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_fac, env, args, acc);
}

static const struct lguard guards_op_pow[] = {
    {.condition= lbi_cond_is_numeric, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_x_is_ul,    .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_op_pow = {
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
int lbuiltin_op_pow(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_op_pow, env, args, acc);
}

static const struct lguard guards_head[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_head = {
    .symbol       = "head",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_head[0],
    .guardc       = LENGTH(guards_head),
    .op_all       = lbi_func_head,
};
int lbuiltin_head(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_head, env, args, acc);
}

static const struct lguard guards_tail[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_tail = {
    .symbol       = "tail",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_tail[0],
    .guardc       = LENGTH(guards_tail),
    .op_all       = lbi_func_tail,
};
int lbuiltin_tail(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_tail, env, args, acc);
}

static const struct lguard guards_init[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
    {.condition= lbi_cond_list,  .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_init = {
    .symbol       = "init",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_init[0],
    .guardc       = LENGTH(guards_init),
    .op_all       = lbi_func_init,
};
int lbuiltin_init(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_init, env, args, acc);
}

static const struct lguard guards_cons[] = {
    {.condition= lbi_cond_qexpr, .argn= 2, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_cons = {
    .symbol       = "cons",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_cons[0],
    .guardc       = LENGTH(guards_cons),
    .op_all       = lbi_func_cons,
};
int lbuiltin_cons(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_cons, env, args, acc);
}

static const struct lguard guards_len[] = {
    {.condition= lbi_cond_qexpr, .argn= 1, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_len = {
    .symbol       = "len",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_len[0],
    .guardc       = LENGTH(guards_len),
    .op_all       = lbi_func_len,
};
int lbuiltin_len(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_len, env, args, acc);
}

static const struct lguard guards_join[] = {
    {.condition= lbi_cond_qexpr, .error= LERR_BAD_OPERAND},
};
const struct ldescriptor lbi_descriptor_join = {
    .symbol       = "join",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_join[0],
    .guardc       = LENGTH(guards_join),
    .op_all       = lbi_func_join,
};
int lbuiltin_join(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_join, env, args, acc);
}

static const struct lguard guards_list[] = {
};
const struct ldescriptor lbi_descriptor_list = {
    .symbol       = "list",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_list[0],
    .guardc       = LENGTH(guards_list),
    .op_all       = lbi_func_list,
};
int lbuiltin_list(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_list, env, args, acc);
}

static const struct lguard guards_eval[] = {
};
const struct ldescriptor lbi_descriptor_eval = {
    .symbol       = "eval",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_eval[0],
    .guardc       = LENGTH(guards_eval),
    .op_all       = lbi_func_eval,
};
int lbuiltin_eval(struct lenv* env, const struct lval* args, struct lval* acc) {
    return lbuiltin_exec(&lbi_descriptor_eval, env, args, acc);
}
