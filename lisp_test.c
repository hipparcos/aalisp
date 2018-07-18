#include "lisp.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include "vendor/mini-gmp/mini-gmp.h"

#include "lval.h"

#include "vendor/snow/snow/snow.h"

/** Test fixtures */
const long fac20 = 2432902008176640000;
mpz_t bn_fac21;

struct lval* result;
struct lval* expected;

#define it_pass(input, output) \
    it(input" == "#output, { \
        output; \
        assert( lisp_eval(input, result)); \
        assert(lval_are_equal(result, expected)); \
    })
#define it_fail(input, output) \
    it(input" == "#output, { \
        output; \
        assert(!lisp_eval(input, result)); \
        assert(lval_are_equal(result, expected)); \
    })

describe(lisp_eval, {
    before_each({
        result = lval_alloc();
        expected = lval_alloc();

        mpz_init_set_ui(bn_fac21, fac20);
        mpz_mul_si(bn_fac21, bn_fac21, 21);
    });
    after_each({
        lval_free(result);
        lval_free(expected);

        mpz_clear(bn_fac21);
    });

    /* happy path */
    it_pass("+ 1 1",                     lval_mut_num(expected, 2));
    it_pass("+ 1",                       lval_mut_num(expected, 1));
    it_pass("+ 1 1 1 1 1 1",             lval_mut_num(expected, 6));
    it_pass("/ 10 0",                    lval_mut_err(expected, LERR_DIV_ZERO));
    it_pass("+ 1 1.0",                   lval_mut_dbl(expected, 2.0));
    it_pass("! 21",                      lval_mut_bignum(expected, bn_fac21));
    it_pass("* 10 (- 20 10)",            lval_mut_num(expected, 100));
    /* nested */
    it_pass("- (! 21) (! 21) 2.0 1",     lval_mut_dbl(expected, -3.0));
    it_pass("- 2.0 1 (- (! 21) (! 21))", lval_mut_dbl(expected, 1.0));

    /* should not be in grammar */
    it_fail("",          lval_mut_err(expected, LERR_EVAL));
    /* it_fail("1 + 1",     lval_mut_err(expected, LERR_EVAL)); */
    it_fail("gibberish", lval_mut_err(expected, LERR_EVAL));

});

snow_main();
