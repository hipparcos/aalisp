#include "lbuiltin.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "vendor/snow/snow/snow.h"

#define LENGTH(array) sizeof(array)/sizeof(array[0])

/* Globals. */
const long fac20 = 2432902008176640000;
mpz_t bn_maxlong_succx2;
mpz_t bn_maxlong_succ;
mpz_t bn_maxlong;
mpz_t bn_maxlong_pred;
mpz_t bn_21;
mpz_t bn_20;
mpz_t bn_8;
mpz_t bn_one;
mpz_t bn_zero;
mpz_t bn_minlong_succ;
mpz_t bn_minlong;
mpz_t bn_minlong_pred;
mpz_t bn_minlong_predx2;
mpz_t bn_10pow1;
mpz_t bn_10pow10;
mpz_t bn_10pow11;
mpz_t bn_10pow100;
mpz_t bn_fac21;

#define test_helper_builtin_pass(op, \
            mut_func_x, x, \
            mut_func_y, y, \
            mut_func_e, expected) \
    test_helper_builtin(op, true, mut_func_x, x, mut_func_y, y, mut_func_e, expected)

#define test_helper_builtin_fail(op, \
            mut_func_x, x, \
            mut_func_y, y, \
            mut_func_e, expected) \
    test_helper_builtin(op, false, mut_func_x, x, mut_func_y, y, mut_func_e, expected)

#define test_helper_builtin(op, must_pass, \
            mut_func_x, x, \
            mut_func_y, y, \
            mut_func_e, expected) \
    struct lval* a = lval_alloc(); struct lval* b = lval_alloc(); \
    struct lval* r = lval_alloc(); struct lval* e = lval_alloc(); \
    mut_func_x(a, x); \
    mut_func_y(b, y); \
    lval_mut_nil(r); \
    mut_func_e(e, expected); \
    if (must_pass) { assert( lsym_exec(op, a, b, r)); } \
    else {           assert(!lsym_exec(op, a, b, r)); } \
    assert(lval_are_equal(r, e)); \
    lval_free(e); lval_free(r); lval_free(b); lval_free(a); \

/** discard is a helper used to do nothing instead of mut_func_y. */
static void discard(struct lval* v, void* i) {
    (void)(v);
    (void)(i);
}

describe(builtin, {
    before_each({
        mpz_init_set_si(bn_maxlong, LONG_MAX);
        mpz_init_set_si(bn_maxlong_succ, LONG_MAX);
        mpz_add_ui(bn_maxlong_succ, bn_maxlong_succ, 1);
        mpz_init_set_si(bn_maxlong_pred, LONG_MAX);
        mpz_sub_ui(bn_maxlong_pred, bn_maxlong_pred, 1);
        mpz_init_set_si(bn_maxlong_succx2, LONG_MAX);
        mpz_add(bn_maxlong_succx2, bn_maxlong_succ, bn_maxlong_succ);

        mpz_init_set_si(bn_21, 21);
        mpz_init_set_si(bn_20, 20);
        mpz_init_set_si(bn_8, 8);
        mpz_init_set_si(bn_one, 1);
        mpz_init_set_si(bn_zero, 0);

        mpz_init_set_si(bn_minlong, LONG_MIN);
        mpz_init_set_si(bn_minlong_succ, LONG_MIN);
        mpz_add_ui(bn_minlong_succ, bn_minlong_succ, 1);
        mpz_init_set_si(bn_minlong_pred, LONG_MIN);
        mpz_sub_ui(bn_minlong_pred, bn_minlong_pred, 1);
        mpz_init_set_si(bn_minlong_predx2, LONG_MIN);
        mpz_sub(bn_minlong_predx2, bn_minlong_pred, bn_minlong_pred);

        mpz_init(bn_10pow1);
        mpz_set_ui(bn_10pow1, 10);
        mpz_init(bn_10pow10);
        mpz_ui_pow_ui(bn_10pow10, 10, 10);
        mpz_init(bn_10pow11);
        mpz_ui_pow_ui(bn_10pow11, 10, 11);
        mpz_init(bn_10pow100);
        mpz_ui_pow_ui(bn_10pow100, 10, 100);

        // fac(20) * 21.
        mpz_init_set_ui(bn_fac21, fac20);
        mpz_mul_si(bn_fac21, bn_fac21, 21);
    });

    after_each({
        mpz_clear(bn_maxlong_succx2);
        mpz_clear(bn_maxlong_succ);
        mpz_clear(bn_maxlong);
        mpz_clear(bn_maxlong_pred);
        mpz_clear(bn_21);
        mpz_clear(bn_20);
        mpz_clear(bn_8);
        mpz_clear(bn_one);
        mpz_clear(bn_zero);
        mpz_clear(bn_minlong_succ);
        mpz_clear(bn_minlong);
        mpz_clear(bn_minlong_pred);
        mpz_clear(bn_minlong_predx2);
        mpz_clear(bn_10pow1);
        mpz_clear(bn_10pow10);
        mpz_clear(bn_10pow11);
        mpz_clear(bn_10pow100);
        mpz_clear(bn_fac21);
    });

    subdesc(op_add, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_num, 1,
                lval_mut_num, 1,
                lval_mut_num, 2);
        });
        it("passes for LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_dbl, 1.0,
                lval_mut_dbl, 1.0,
                lval_mut_dbl, 2.0);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_bignum, bn_maxlong_succx2);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_num,    1,
                lval_mut_bignum, bn_maxlong,
                lval_mut_bignum, bn_maxlong_succ);
        });
        it("passes for LVAL_DBL and LVAL_NUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_num, 1,
                lval_mut_dbl, 1.0,
                lval_mut_dbl, 2.0);
        });
        it("passes for LVAL_DBL and LVAL_BIGNUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_add,
                lval_mut_dbl,    1.0,
                lval_mut_bignum, bn_one,
                lval_mut_dbl,    2.0);
        });

    });

    subdesc(op_sub, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_num, 2,
                lval_mut_num, 1,
                lval_mut_num, 1);
        });
        it("passes for LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_dbl, 2.0,
                lval_mut_dbl, 1.0,
                lval_mut_dbl, 1.0);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_bignum, bn_maxlong,
                lval_mut_bignum, bn_one);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_num,    1,
                lval_mut_bignum, bn_maxlong);
        });
        it("passes for LVAL_DBL and LVAL_NUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_num,   2,
                lval_mut_dbl, 1.0,
                lval_mut_dbl, 1.0);
        });
        it("passes for LVAL_DBL and LVAL_BIGNUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_sub,
                lval_mut_dbl,    2.0,
                lval_mut_bignum, bn_one,
                lval_mut_dbl,    1.0);
        });

    });

    subdesc(op_mul, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_num, 10,
                lval_mut_num, 20,
                lval_mut_num, 200);
        });
        it("passes for LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_dbl, 10.0,
                lval_mut_dbl, 20.0,
                lval_mut_dbl, 200.0);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_bignum, bn_10pow1,
                lval_mut_bignum, bn_10pow10,
                lval_mut_bignum, bn_10pow11);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_bignum, bn_10pow10,
                lval_mut_num,    10,
                lval_mut_bignum, bn_10pow11);
        });
        it("passes for LVAL_DBL and LVAL_NUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_num, 20,
                lval_mut_dbl, 10.0,
                lval_mut_dbl, 200.0);
        });
        it("passes for LVAL_DBL and LVAL_BIGNUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_mul,
                lval_mut_dbl,    2.0,
                lval_mut_bignum, bn_10pow1,
                lval_mut_dbl,    20.0);
        });

    });

    subdesc(op_div, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_num, 20,
                lval_mut_num, 10,
                lval_mut_num, 2);
        });
        it("passes for LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_dbl, 20.0,
                lval_mut_dbl, 10.0,
                lval_mut_dbl, 2.0);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_bignum, bn_10pow11,
                lval_mut_bignum, bn_10pow1,
                lval_mut_bignum, bn_10pow10);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_bignum, bn_10pow11,
                lval_mut_num,    10,
                lval_mut_bignum, bn_10pow10);
        });
        it("passes for LVAL_DBL and LVAL_NUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_num, 20,
                lval_mut_dbl, 10.0,
                lval_mut_dbl, 2.0);
        });
        it("passes for LVAL_DBL and LVAL_BIGNUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_div,
                lval_mut_dbl,    20.0,
                lval_mut_bignum, bn_10pow1,
                lval_mut_dbl,    2.0);
        });
        it("fails when the divisor is 0 (LVAL_NUM)", {
            test_helper_builtin_fail(lbuiltin_op_div,
                lval_mut_num, 20,
                lval_mut_num, 0,
                lval_mut_err, LERR_DIV_ZERO);
        });
        it("fails when the divisor is 0 (LVAL_DBL)", {
            test_helper_builtin_fail(lbuiltin_op_div,
                lval_mut_dbl, 20.0,
                lval_mut_dbl, 0.0,
                lval_mut_err, LERR_DIV_ZERO);
        });
        it("fails when the divisor is 0 (LVAL_DBL & LVAL_BIGNUM", {
            test_helper_builtin_fail(lbuiltin_op_div,
                lval_mut_dbl,    20.0,
                lval_mut_bignum, bn_zero,
                lval_mut_err,    LERR_DIV_ZERO);
        });

    });

    subdesc(op_mod, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_mod,
                lval_mut_num, 20,
                lval_mut_num, 18,
                lval_mut_num, 2);
        });
        it("fails for LVAL_DBL", {
            test_helper_builtin_fail(lbuiltin_op_mod,
                lval_mut_dbl, 20.0,
                lval_mut_dbl, 18.0,
                lval_mut_err, LERR_BAD_OPERAND);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_mod,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_bignum, bn_maxlong,
                lval_mut_bignum, bn_one);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_mod,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_num,    LONG_MAX,
                lval_mut_bignum, bn_one);
        });
        it("fails when the modisor is 0 (LVAL_NUM)", {
            test_helper_builtin_fail(lbuiltin_op_mod,
                lval_mut_num, 20,
                lval_mut_num, 0,
                lval_mut_err, LERR_DIV_ZERO);
        });
        it("fails when the modisor is 0 (LVAL_DBL & LVAL_BIGNUM", {
            test_helper_builtin_fail(lbuiltin_op_mod,
                lval_mut_bignum, bn_maxlong_succ,
                lval_mut_bignum, bn_zero,
                lval_mut_err,    LERR_DIV_ZERO);
        });

    });

    subdesc(op_fac, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_fac,
                lval_mut_num, 20,
                discard, NULL,
                lval_mut_num, fac20);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_fac,
                lval_mut_bignum, bn_21,
                discard, NULL,
                lval_mut_bignum, bn_fac21);
        });
        it("fails for LVAL_DBL", {
            test_helper_builtin_fail(lbuiltin_op_fac,
                lval_mut_dbl, 20.0,
                discard, NULL,
                lval_mut_err, LERR_BAD_OPERAND);
        });
        it("fails for LVAL_NUM < 0", {
            test_helper_builtin_fail(lbuiltin_op_fac,
                lval_mut_num, -20,
                discard, NULL,
                lval_mut_err, LERR_BAD_OPERAND);
        });
        it("fails for 2 operands", {
            test_helper_builtin_fail(lbuiltin_op_fac,
                lval_mut_num, 20,
                lval_mut_num, 20,
                lval_mut_err, LERR_TOO_MANY_ARGS);
        });

    });

    subdesc(op_pow, {

        it("passes for LVAL_NUM", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_num, 2,
                lval_mut_num, 8,
                lval_mut_num, 256);
        });
        it("passes for LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_dbl, 2.0,
                lval_mut_dbl, 8.0,
                lval_mut_dbl, 256.0);
        });
        it("passes for LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_bignum, bn_10pow10,
                lval_mut_bignum, bn_10pow1,
                lval_mut_bignum, bn_10pow100);
        });
        it("passes for LVAL_BIGNUM and LVAL_NUM casted to LVAL_BIGNUM", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_bignum, bn_10pow10,
                lval_mut_num,    10,
                lval_mut_bignum, bn_10pow100);
        });
        it("passes for LVAL_DBL and LVAL_NUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_num, 2,
                lval_mut_dbl, 8.0,
                lval_mut_dbl, 256.0);
        });
        it("passes for LVAL_DBL and LVAL_BIGNUM casted to LVAL_DBL", {
            test_helper_builtin_pass(lbuiltin_op_pow,
                lval_mut_dbl,    2.0,
                lval_mut_bignum, bn_8,
                lval_mut_dbl,    256.0);
        });

    });

});

snow_main();