#include "lbuiltin.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "vendor/snow/snow/snow.h"

#include "lval.h"
#include "lenv.h"
#include "lfunc.h"

/* Globals. */
const long fac20 = 2432902008176640000;

#define test_pass(func, msg, ...) \
    it("passes for "msg, { \
        struct lval* args = lval_alloc(); defer(lval_free(args)); \
        lval_mut_qexpr(args); \
        struct lval* expected = lval_alloc(); defer(lval_free(expected)); \
        struct lval* got = lval_alloc(); defer(lval_free(got)); \
        __VA_ARGS__ \
        struct lenv* env = lenv_alloc(); \
        defer(lenv_free(env)); \
        lenv_default(env); \
        assert(0 == lfunc_exec(func, env, args, got)); \
        assert(lval_are_equal(got, expected)); \
    });

#define test_fail(func, msg, ...) \
    it("fails for "msg, { \
        struct lval* args = lval_alloc(); defer(lval_free(args)); \
        lval_mut_qexpr(args); \
        struct lval* expected = lval_alloc(); defer(lval_free(expected)); \
        struct lval* got = lval_alloc(); defer(lval_free(got)); \
        __VA_ARGS__ \
        struct lenv* env = lenv_alloc(); \
        defer(lenv_free(env)); \
        lenv_default(env); \
        assert(0 != lfunc_exec(func, env, args, got)); \
        assert(lval_are_equal(got, expected)); \
    });

#define push_sym(args, sym) \
    do { \
        struct lval* x = lval_alloc(); \
        lval_mut_sym(x, sym); \
        lval_push(args, x); \
        lval_free(x); \
    } while (0);

#define push_num(args, num) \
    do { \
        struct lval* x = lval_alloc(); \
        lval_mut_num(x, num); \
        lval_push(args, x); \
        lval_free(x); \
    } while (0);

#define push_dbl(args, dbl) \
    do { \
        struct lval* x = lval_alloc(); \
        lval_mut_dbl(x, dbl); \
        lval_push(args, x); \
        lval_free(x); \
    } while (0);

#define push_bignum(args, ul) \
    do { \
        mpz_t bn; \
        mpz_init_set_ui(bn, ul); \
        struct lval* x = lval_alloc(); \
        lval_mut_bignum(x, bn); \
        lval_push(args, x); \
        lval_free(x); \
        mpz_clear(bn); \
    } while (0);

#define push_bignum_mul(args, ul, si) \
    do { \
        mpz_t bn; \
        mpz_init_set_ui(bn, ul); \
        mpz_mul_si(bn, bn, si); \
        struct lval* x = lval_alloc(); \
        lval_mut_bignum(x, bn); \
        lval_push(args, x); \
        lval_free(x); \
        mpz_clear(bn); \
    } while (0);

#define mut_bignum(arg, ul) \
    do { \
        mpz_t bn; \
        mpz_init_set_ui(bn, ul); \
        lval_mut_bignum(arg, bn); \
        mpz_clear(bn); \
    } while(0);

#define mut_bignum_add(arg, ul, si) \
    do { \
        mpz_t bn; \
        mpz_init_set_ui(bn, ul); \
        if (si > 0) { \
            mpz_add_ui(bn, bn, si); \
        } else { \
            mpz_sub_ui(bn, bn, -si); \
        } \
        lval_mut_bignum(arg, bn); \
        mpz_clear(bn); \
    } while(0);

#define mut_bignum_mul(arg, ul, si) \
    do { \
        mpz_t bn; \
        mpz_init_set_ui(bn, ul); \
        mpz_mul_si(bn, bn, si); \
        lval_mut_bignum(arg, bn); \
        mpz_clear(bn); \
    } while(0);

describe(builtin, {

    subdesc(op_add, {
        test_pass(&lbuiltin_op_add, "LVAL_NUM", {
            push_num(args, 1);
            push_num(args, 2);
            lval_mut_num(expected, 3);
        });
        test_pass(&lbuiltin_op_add, "LVAL_DBL", {
            push_dbl(args, 1.0);
            push_dbl(args, 2.0);
            lval_mut_dbl(expected, 3.0);
        });
        test_pass(&lbuiltin_op_add, "LVAL_BIGNUM", {
            push_bignum(args, ULONG_MAX);
            push_bignum(args, 2);
            mut_bignum_add(expected, ULONG_MAX, 2);
        });
        test_pass(&lbuiltin_op_add, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 2);
            push_bignum(args, ULONG_MAX);
            mut_bignum_add(expected, ULONG_MAX, 2);
        });
        test_pass(&lbuiltin_op_add, "LVAL_DBL & LVAL_NUM casted to LVAL_DBL", {
            push_num(args, 1);
            push_dbl(args, 2.0);
            lval_mut_dbl(expected, 3.0);
        });
        test_pass(&lbuiltin_op_add, "LVAL_DBL & LVAL_BIGNUM casted to LVAL_DBL", {
            push_dbl(args, 1.0);
            push_bignum(args, 2);
            lval_mut_dbl(expected, 3.0);
        });
    });

    subdesc(op_sub, {
        test_pass(&lbuiltin_op_sub, "LVAL_NUM", {
            push_num(args, 3);
            push_num(args, 1);
            lval_mut_num(expected, 2);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_DBL", {
            push_dbl(args, 3.0);
            push_dbl(args, 1.0);
            lval_mut_dbl(expected, 2.0);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_BIGNUM", {
            push_bignum(args, ULONG_MAX);
            push_bignum(args, 2);
            mut_bignum_add(expected, ULONG_MAX, -2);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 3);
            push_bignum(args, 1);
            mut_bignum_add(expected, 2, 0);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_DBL & LVAL_NUM casted to LVAL_DBL", {
            push_num(args, 3);
            push_dbl(args, 1.0);
            lval_mut_dbl(expected, 2.0);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_DBL & LVAL_BIGNUM casted to LVAL_DBL", {
            push_dbl(args, 3.0);
            push_bignum(args, 1);
            lval_mut_dbl(expected, 2.0);
        });
    });

    subdesc(op_sub_unary, {
        test_pass(&lbuiltin_op_sub, "LVAL_NUM", {
            push_num(args, 2);
            lval_mut_num(expected, -2);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_DBL", {
            push_dbl(args, 2.0);
            lval_mut_dbl(expected, -2.0);
        });
        test_pass(&lbuiltin_op_sub, "LVAL_BIGNUM", {
            push_bignum(args, 2);
            mut_bignum_add(expected, 0, -2);
        });
    });

    subdesc(op_mul, {
        test_pass(&lbuiltin_op_mul, "LVAL_NUM", {
            push_num(args, 10);
            push_num(args, 20);
            lval_mut_num(expected, 200);
        });
        test_pass(&lbuiltin_op_mul, "LVAL_DBL", {
            push_dbl(args, 10.0);
            push_dbl(args, 20.0);
            lval_mut_dbl(expected, 200.0);
        });
        test_pass(&lbuiltin_op_mul, "LVAL_BIGNUM", {
            push_bignum(args, ULONG_MAX);
            push_bignum(args, 10);
            mut_bignum_mul(expected, ULONG_MAX, 10);
        });
        test_pass(&lbuiltin_op_mul, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 2);
            push_bignum(args, ULONG_MAX);
            mut_bignum_mul(expected, ULONG_MAX, 2);
        });
        test_pass(&lbuiltin_op_mul, "LVAL_DBL & LVAL_NUM casted to LVAL_DBL", {
            push_num(args, 10);
            push_dbl(args, 20.0);
            lval_mut_dbl(expected, 200.0);
        });
        test_pass(&lbuiltin_op_mul, "LVAL_DBL & LVAL_BIGNUM casted to LVAL_DBL", {
            push_dbl(args, 10.0);
            push_bignum(args, 20);
            lval_mut_dbl(expected, 200.0);
        });
    });

    subdesc(op_div, {
        test_pass(&lbuiltin_op_div, "LVAL_NUM", {
            push_num(args, 20);
            push_num(args, 10);
            lval_mut_num(expected, 2);
        });
        test_pass(&lbuiltin_op_div, "LVAL_DBL", {
            push_dbl(args, 20.0);
            push_dbl(args, 10.0);
            lval_mut_dbl(expected, 2.0);
        });
        test_pass(&lbuiltin_op_div, "LVAL_BIGNUM", {
            push_bignum_mul(args, ULONG_MAX, 10);
            push_bignum(args, 10);
            mut_bignum_mul(expected, ULONG_MAX, 1);
        });
        test_pass(&lbuiltin_op_div, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 200);
            push_bignum(args, 10);
            mut_bignum(expected, 20);
        });
        test_pass(&lbuiltin_op_div, "LVAL_DBL & LVAL_NUM casted to LVAL_DBL", {
            push_num(args, 200);
            push_dbl(args, 10.0);
            lval_mut_dbl(expected, 20.0);
        });
        test_pass(&lbuiltin_op_div, "LVAL_DBL & LVAL_BIGNUM casted to LVAL_DBL", {
            push_dbl(args, 200.0);
            push_bignum(args, 10);
            lval_mut_dbl(expected, 20.0);
        });
        test_fail(&lbuiltin_op_div, "divisor of type LVAL_NUM = 0", {
            push_num(args, 200);
            push_num(args, 0);
            lval_mut_err(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_div, "divisor of type LVAL_BIGNUM = 0", {
            push_bignum(args, 200);
            push_bignum(args, 0);
            lval_mut_err(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_div, "divisor of type LVAL_DBL = 0", {
            push_dbl(args, 200);
            push_dbl(args, 0.0);
            lval_mut_err(expected, LERR_DIV_ZERO);
        });
    });

    subdesc(op_mod, {
        test_pass(&lbuiltin_op_mod, "LVAL_NUM", {
            push_num(args, 10);
            push_num(args, 8);
            lval_mut_num(expected, 2);
        });
        test_fail(&lbuiltin_op_mod, "LVAL_DBL", {
            push_dbl(args, 10.0);
            push_dbl(args, 8.0);
            lval_mut_err(expected, LERR_BAD_OPERAND);
        });
        test_pass(&lbuiltin_op_mod, "LVAL_BIGNUM", {
            push_bignum(args, 10);
            push_bignum(args, 8);
            mut_bignum(expected, 2);
        });
        test_pass(&lbuiltin_op_mod, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 10);
            push_bignum(args, 8);
            mut_bignum(expected, 2);
        });
        test_fail(&lbuiltin_op_mod, "divisor of type LVAL_NUM = 0", {
            push_num(args, 10);
            push_num(args, 0);
            lval_mut_err(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_mod, "divisor of type LVAL_BIGNUM = 0", {
            push_bignum(args, 10);
            push_bignum(args, 0);
            lval_mut_err(expected, LERR_DIV_ZERO);
        });
    });

    subdesc(op_fac, {
        test_pass(&lbuiltin_op_fac, "LVAL_NUM", {
            push_num(args, 20);
            lval_mut_num(expected, fac20);
        });
        test_fail(&lbuiltin_op_fac, "LVAL_DBL", {
            push_dbl(args, 20.0);
            lval_mut_err(expected, LERR_BAD_OPERAND);
        });
        test_pass(&lbuiltin_op_fac, "LVAL_BIGNUM", {
            push_bignum(args, 20);
            mut_bignum(expected, fac20);
        });
        test_pass(&lbuiltin_op_fac, "LVAL_NUM which overflows to LVAL_BIGNUM", {
            push_num(args, 21);
            mut_bignum_mul(expected, fac20, 21);
        });
        test_fail(&lbuiltin_op_fac, "LVAL_NUM < 0", {
            push_num(args, -20);
            lval_mut_err(expected, LERR_BAD_OPERAND);
        });
        test_fail(&lbuiltin_op_fac, "number of operands > 1", {
            push_num(args, 9);
            push_num(args, 8);
            lval_mut_err(expected, LERR_TOO_MANY_ARGS);
        });
    });

    subdesc(op_pow, {
        test_pass(&lbuiltin_op_pow, "LVAL_NUM", {
            push_num(args, 2);
            push_num(args, 8);
            lval_mut_num(expected, 256);
        });
        test_pass(&lbuiltin_op_pow, "LVAL_DBL", {
            push_dbl(args, 2);
            push_dbl(args, 8);
            lval_mut_dbl(expected, 256.0);
        });
        test_pass(&lbuiltin_op_pow, "LVAL_BIGNUM", {
            push_bignum(args, 2);
            push_bignum(args, 8);
            mut_bignum(expected, 256);
        });
        test_pass(&lbuiltin_op_pow, "LVAL_BIGNUM & LVAL_NUM casted to LVAL_BIGNUM", {
            push_num(args, 2);
            push_bignum(args, 8);
            mut_bignum(expected, 256);
        });
        test_pass(&lbuiltin_op_pow, "LVAL_DBL & LVAL_NUM casted to LVAL_DBL", {
            push_num(args, 2);
            push_dbl(args, 8.0);
            lval_mut_dbl(expected, 256.0);
        });
        test_pass(&lbuiltin_op_pow, "LVAL_DBL & LVAL_BIGNUM casted to LVAL_DBL", {
            push_dbl(args, 2.0);
            push_bignum(args, 8);
            lval_mut_dbl(expected, 256.0);
        });
    });

    subdesc(func_head, {
        test_pass(&lbuiltin_head, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 1);
        });
    });

    subdesc(func_tail, {
        test_pass(&lbuiltin_tail, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 2);
            push_num(expected, 3);
        });
    });

    subdesc(func_init, {
        test_pass(&lbuiltin_init, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
        });
    });

    subdesc(func_last, {
        test_pass(&lbuiltin_last, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_cons, {
        test_pass(&lbuiltin_cons, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 1);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
        });

        test_pass(&lbuiltin_cons, "prepending a Q-Expression", {
            struct lval* qexpr1 = lval_alloc();
            lval_mut_qexpr(qexpr1);
            push_num(qexpr1, 2);
            push_num(qexpr1, 3);
            struct lval* qexpr2 = lval_alloc();
            lval_mut_qexpr(qexpr2);
            push_num(qexpr2, 1);
            lval_push(args, qexpr2);
            lval_push(args, qexpr1);
            lval_free(qexpr1);
            lval_mut_qexpr(expected);
            lval_push(expected, qexpr2);
            push_num(expected, 2);
            push_num(expected, 3);
            lval_free(qexpr2);
        });
    });

    subdesc(func_len, {
        test_pass(&lbuiltin_len, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_join, {
        test_pass(&lbuiltin_join, "happy path", {
            struct lval* qexpr1 = lval_alloc();
            lval_mut_qexpr(qexpr1);
            push_num(qexpr1, 1);
            push_num(qexpr1, 2);
            lval_push(args, qexpr1);
            lval_free(qexpr1);
            struct lval* qexpr2 = lval_alloc();
            lval_mut_qexpr(qexpr2);
            push_num(qexpr2, 3);
            push_num(qexpr2, 4);
            lval_push(args, qexpr2);
            lval_free(qexpr2);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
        });

        test_pass(&lbuiltin_join, "no arguments given (returns {})", {
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_list, {
        test_pass(&lbuiltin_list, "happy path", {
            push_num(args, 1);
            push_num(args, 2);
            push_num(args, 3);
            push_num(args, 4);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
        });

        test_pass(&lbuiltin_list, "no arguments given (returns {})", {
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_eval, {
        test_pass(&lbuiltin_eval, "happy path", {
            struct lval* sexpr = lval_alloc();
            lval_mut_sexpr(sexpr);
            push_sym(sexpr, "+");
            push_num(sexpr, 1);
            push_num(sexpr, 2);
            lval_push(args, sexpr);
            lval_free(sexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_def, {
        test_pass(&lbuiltin_def, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_dup(expected, qexpr);
            lval_free(qexpr);
        });

        test_fail(&lbuiltin_def, "symbols > args", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            lval_free(qexpr);
            lval_mut_err(expected, LERR_TOO_FEW_ARGS);
        });

        test_fail(&lbuiltin_def, "symbols < args", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_free(qexpr);
            lval_mut_err(expected, LERR_TOO_MANY_ARGS);
        });
    });

    subdesc(func_put, {
        test_pass(&lbuiltin_put, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_dup(expected, qexpr);
            lval_free(qexpr);
        });
    });

});

snow_main();
