#include "builtin.h"

/* C files included here. This file was split for clarity only. */
#include "builtin_condition.c"
#include "builtin_typed_operator.c"

struct lval lisp_builtin_op(
    const struct op_descriptor descriptor,
    const struct lval x, struct lval y
) {

    if (!descriptor.unary && y.type == LVAL_NIL) {
        y = *descriptor.neutral;
    }

    /* Guards */
    for (int i = 0; i < descriptor.guardc; i++) {
        if ((descriptor.guards[i]->condition)(x, y)) {
            return lval_err(descriptor.guards[i]->error);
        }
    }

    /* Eval: double */
    if (cnd_either_is_dbl(x, y)) {
        double a = lval_as_dbl(x);
        double b = lval_as_dbl(y);
        return lval_dbl(descriptor.op_dbl(a, b));
    }
    /* Eval: bignum */
    if (cnd_either_is_bignum(x, y)) {
        mpz_t a, b;
        lval_as_bignum(x, a);
        lval_as_bignum(y, b);
        mpz_t r;
        mpz_init(r);
        descriptor.op_bignum(r, a, b);
        mpz_clear(a);
        mpz_clear(b);
        struct lval ret = lval_bignum(r);
        mpz_clear(r);
        return ret;
    }
    /* Eval: num */
    if (cnd_are_num(x, y)) {
        long a = x.data.num;
        long b = y.data.num;
        if (descriptor.cnd_overflow && descriptor.cnd_overflow(a, b)) {
            mpz_t bna, bnb;
            lval_as_bignum(x, bna);
            lval_as_bignum(y, bnb);
            struct lval vbna = lval_bignum(bna);
            struct lval vbnb = lval_bignum(bnb);
            struct lval res = lisp_builtin_op(descriptor, vbna, vbnb);
            mpz_clear(bna);
            mpz_clear(bnb);
            lval_clear(&vbna);
            lval_clear(&vbnb);
            return res;
        }
        return lval_num(descriptor.op_num(a, b));
    }

    return lval_err(LERR_EVAL);
}

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Guards: instanciation. */
static const struct guard guard_x_is_negative = {
    .condition= cnd_x_is_neg,
    .error= LERR_BAD_NUM
};
static const struct guard guard_y_is_negative = {
    .condition= cnd_y_is_neg,
    .error= LERR_BAD_NUM
};
static const struct guard guard_x_too_big = {
    .condition= cnd_x_too_big_for_ul,
    .error= LERR_BAD_NUM
};
static const struct guard guard_y_too_big = {
    .condition= cnd_y_too_big_for_ul,
    .error= LERR_BAD_NUM
};
static const struct guard guard_div_by_zero = {
    .condition= cnd_y_is_zero,
    .error= LERR_DIV_ZERO
};
static const struct guard guard_either_is_double = {
    .condition= cnd_either_is_dbl,
    .error= LERR_BAD_NUM
};
static const struct guard guard_dbl_and_bignum = {
    .condition= cnd_dbl_and_bignum,
    .error= LERR_BAD_NUM
};

/* Operator: declaration */
static const struct guard* guards_op_add[] = {
    &guard_dbl_and_bignum
};
const struct op_descriptor builtin_op_add = {
    .symbol       = "+",
    .guards       = guards_op_add,
    .guardc       = LENGTH(guards_op_add),
    .neutral      = &lzero,
    .op_num       = op_num_add,
    .cnd_overflow = cnd_num_add_overflow,
    .op_bignum    = mpz_add,
    .op_dbl       = op_dbl_add
};

static const struct guard* guards_op_sub[] = {
    &guard_dbl_and_bignum
};
const struct op_descriptor builtin_op_sub = {
    .symbol       = "-",
    .guards       = guards_op_sub,
    .guardc       = LENGTH(guards_op_sub),
    .neutral      = &lzero,
    .op_num       = op_num_sub,
    .cnd_overflow = cnd_num_sub_overflow,
    .op_bignum    = mpz_sub,
    .op_dbl       = op_dbl_sub
};

static const struct guard* guards_op_mul[] = {
    &guard_dbl_and_bignum
};
const struct op_descriptor builtin_op_mul = {
    .symbol       = "*",
    .guards       = guards_op_mul,
    .guardc       = LENGTH(guards_op_mul),
    .neutral      = &lone,
    .op_num       = op_num_mul,
    .cnd_overflow = cnd_num_mul_overflow,
    .op_bignum    = mpz_mul,
    .op_dbl       = op_dbl_mul
};

static const struct guard* guards_op_div[] = {
    &guard_div_by_zero,
    &guard_dbl_and_bignum
};
const struct op_descriptor builtin_op_div = {
    .symbol       = "/",
    .guards       = guards_op_div,
    .guardc       = LENGTH(guards_op_div),
    .neutral      = &lone,
    .op_num       = op_num_div,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_fdiv_q,
    .op_dbl       = op_dbl_div,
};

static const struct guard* guards_op_mod[] = {
    &guard_div_by_zero,
    &guard_either_is_double,
    &guard_dbl_and_bignum,
};
const struct op_descriptor builtin_op_mod = {
    .symbol       = "%",
    .guards       = guards_op_mod,
    .guardc       = LENGTH(guards_op_mod),
    .neutral      = &lone,
    .op_num       = op_num_mod,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_mod,
    .op_dbl       = op_dbl_nop
};

static const struct guard* guards_op_fac[] = {
    &guard_either_is_double,
    &guard_x_is_negative,
    &guard_x_too_big
};
const struct op_descriptor builtin_op_fac = {
    .symbol       = "!",
    .unary        = true,
    .guards       = guards_op_fac,
    .guardc       = LENGTH(guards_op_fac),
    .op_num       = op_num_fact,
    .cnd_overflow = cnd_num_fact_overflow,
    .op_bignum    = op_bignum_fac,
    .op_dbl       = op_dbl_nop
};

static const struct guard *guards_op_pow[] = {
    &guard_dbl_and_bignum,
    &guard_y_is_negative,
    &guard_y_too_big
};
const struct op_descriptor builtin_op_pow = {
    .symbol       = "^",
    .guards       = guards_op_pow,
    .guardc       = LENGTH(guards_op_pow),
    .neutral      = &lzero,
    .op_num       = op_num_pow,
    .cnd_overflow = cnd_num_pow_overflow,
    .op_bignum    = op_bignum_pow,
    .op_dbl       = op_dbl_pow
};
