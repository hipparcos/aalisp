#include "lbuiltin.h"

#include <limits.h>

#include "lbuiltin_test.h"

const long fac20 = 2432902008176640000;

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
            lval_mut_err_code(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_div, "divisor of type LVAL_BIGNUM = 0", {
            push_bignum(args, 200);
            push_bignum(args, 0);
            lval_mut_err_code(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_div, "divisor of type LVAL_DBL = 0", {
            push_dbl(args, 200);
            push_dbl(args, 0.0);
            lval_mut_err_code(expected, LERR_DIV_ZERO);
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
            lval_mut_err_code(expected, LERR_BAD_OPERAND);
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
            lval_mut_err_code(expected, LERR_DIV_ZERO);
        });
        test_fail(&lbuiltin_op_mod, "divisor of type LVAL_BIGNUM = 0", {
            push_bignum(args, 10);
            push_bignum(args, 0);
            lval_mut_err_code(expected, LERR_DIV_ZERO);
        });
    });

    subdesc(op_fac, {
        test_pass(&lbuiltin_op_fac, "LVAL_NUM", {
            push_num(args, 20);
            lval_mut_num(expected, fac20);
        });
        test_fail(&lbuiltin_op_fac, "LVAL_DBL", {
            push_dbl(args, 20.0);
            lval_mut_err_code(expected, LERR_BAD_OPERAND);
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
            lval_mut_err_code(expected, LERR_BAD_OPERAND);
        });
        test_fail(&lbuiltin_op_fac, "number of operands > 1", {
            push_num(args, 9);
            push_num(args, 8);
            lval_mut_err_code(expected, LERR_TOO_MANY_ARGS);
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

    subdesc(op_eq, {
        test_pass(&lbuiltin_op_eq, "equals values", {
            push_num(args, 2);
            push_num(args, 2);
            lval_mut_bool(expected, true);
        });
        test_pass(&lbuiltin_op_eq, "distinct values", {
            push_num(args, 2);
            push_num(args, 8);
            lval_mut_bool(expected, false);
        });
        test_pass(&lbuiltin_op_eq, "equals values with cast", {
            push_num(args, 2);
            push_dbl(args, 2.0);
            lval_mut_bool(expected, true);
        });
    });

});

snow_main();
