#define _POSIX_C_SOURCE 200809L /* For fmemopen. */

#include "lval.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lbuiltin_test.h"

#include "vendor/mini-gmp/mini-gmp.h"
#include "vendor/snow/snow/snow.h"

#define LENGTH(array) sizeof(array)/sizeof(array[0])

#define to_string(val,out) \
    do { \
        FILE* fm = fmemopen(out, sizeof(out), "w+"); \
        defer(fclose(fm)); \
        lval_print_to(val, fm); \
        fseek(fm, 0, SEEK_SET); \
    } while (0);

describe(lval, {

    it("allocates a lval and frees it", {
        struct lval* v = lval_alloc();
        assert(lval_free(v));
    });

    subdesc(mut, {
        it("mutates a lval to a num", {
            long input = 10;
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, input));
            assert(lval_type(v) == LVAL_NUM);
            assert(lval_free(v));
        });

        it("mutates a lval to a bignum", {
            mpz_t input; /** = 10^100 */
            mpz_init_set_si(input, 10);
            mpz_pow_ui(input, input, 100u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, input));
            assert(lval_type(v) == LVAL_BIGNUM);
            assert(lval_free(v));
            mpz_clear(input);
        });

        it("mutates a lval to a double", {
            double input = 10;
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, input));
            assert(lval_type(v) == LVAL_DBL);
            assert(lval_free(v));
        });

        it("mutates a lval to an error", {
            enum lerr_code input = LERR_EVAL;
            struct lval* v = lval_alloc();
            assert(lval_mut_err_code(v, input));
            assert(lval_type(v) == LVAL_ERR);
            assert(lval_free(v));
        });

        it("mutates a lval to a string", {
            const char* input = "input";
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, input));
            assert(lval_type(v) == LVAL_STR);
            assert(lval_free(v));
        });

        it("mutates a lval to a symbol", {
            const char* input = "input";
            struct lval* v = lval_alloc();
            assert(lval_mut_sym(v, input));
            assert(lval_type(v) == LVAL_SYM);
            assert(lval_free(v));
        });

        it("mutates the same value to a num, a bignum and finally to a str (no leakages)", {
            long inum = 10;
            mpz_t ibn; /** = 10^20 */
            mpz_init_set_si(ibn, 10);
            mpz_pow_ui(ibn, ibn, 20u);
            const char* istr = "str";
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, inum));
            assert(lval_type(v) == LVAL_NUM);
            assert(lval_mut_bignum(v, ibn));
            assert(lval_type(v) == LVAL_BIGNUM);
            assert(lval_mut_str(v, istr));
            assert(lval_type(v) == LVAL_STR);
            assert(lval_free(v));
            mpz_clear(ibn);
        });
    });

    subdesc(as, {
        it("mutates a lval to bool and read it back", {
            struct lval* v = lval_alloc();
            defer(lval_free(v));
            assert(lval_mut_bool(v, true));
            assert(lval_type(v) == LVAL_BOOL);
            assert(lval_as_bool(v));
        });

        it("mutates a lval to a num and read it back", {
            long expected = 10;
            long got = 0;
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, expected));
            assert(lval_type(v) == LVAL_NUM);
            assert(lval_as_num(v, &got));
            assert(got == expected);
            assert(lval_free(v));
        });

        it("mutates a lval to a bignum and read it back", {
            mpz_t expected; /** = 10^100 */
            mpz_init_set_si(expected, 10);
            mpz_pow_ui(expected, expected, 100u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, expected));
            assert(lval_type(v) == LVAL_BIGNUM);
            mpz_t got;
            mpz_init(got);
            assert(lval_as_bignum(v, got));
            assert(mpz_cmp(got, expected) == 0);
            mpz_clear(got);
            assert(lval_free(v));
            mpz_clear(expected);
        });

        it("mutates a lval to a double and read it back", {
            double expected = 10;
            double got = 0;
            double epsilon = 0.000001;
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, expected));
            assert(lval_type(v) == LVAL_DBL);
            assert(lval_as_dbl(v, &got));
            assert(abs(got - expected) < epsilon);
            assert(lval_free(v));
        });

        it("mutates a lval to an error and read it back", {
            enum lerr_code expected = LERR_EVAL;
            enum lerr_code got = 0;
            struct lval* v = lval_alloc();
            assert(lval_mut_err_code(v, expected));
            assert(lval_type(v) == LVAL_ERR);
            assert(lval_as_err_code(v, &got));
            assert(got == expected);
            assert(lval_free(v));
        });

        it("mutates a lval to a string and read it back", {
            const char* expected = "expected";
            const char* got = NULL;
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, expected));
            assert(lval_type(v) == LVAL_STR);
            assert(got = lval_as_str(v));
            assert(strcmp(got, expected) == 0);
            assert(lval_free(v));
        });

        it("mutates a lval to a symbol and read it back", {
            const char* expected = "expected";
            const char* got = NULL;
            struct lval* v = lval_alloc();
            assert(lval_mut_sym(v, expected));
            assert(lval_type(v) == LVAL_SYM);
            assert(got = lval_as_sym(v));
            assert(strcmp(got, expected) == 0);
            assert(lval_free(v));
        });
    });

    subdesc(clear, {
        it("clears the given lval and sets it to LVAL_NIL", {
            long input = 10;
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, input));
            assert(lval_type(v) == LVAL_NUM);
            assert(!lval_is_nil(v));
            assert(lval_clear(v));
            assert(lval_is_nil(v));
            assert(lval_free(v));
        });

        it("clears the given lval of type LVAL_BIGNUM and sets it to LVAL_NIL (no leakage)", {
            mpz_t input; /** = 10^20 */
            mpz_init_set_si(input, 10);
            mpz_pow_ui(input, input, 20u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, input));
            assert(lval_type(v) == LVAL_BIGNUM);
            assert(!lval_is_nil(v));
            assert(lval_clear(v));
            assert(lval_is_nil(v));
            assert(lval_free(v));
            mpz_clear(input);
        });

        it("clears the given lval of type LVAL_STR and sets it to LVAL_NIL (no leakage)", {
            const char* input = "input";
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, input));
            assert(lval_type(v) == LVAL_STR);
            assert(!lval_is_nil(v));
            assert(lval_clear(v));
            assert(lval_is_nil(v));
            assert(lval_free(v));
        });
    });

    subdesc(dup, {
        it("duplicates handles", {
            struct lval* dest = lval_alloc();
            defer(lval_free(dest));
            struct lval* src = lval_alloc();
            lval_mut_num(src, 10);
            assert(lval_dup(dest, src));
            assert(dest->data == src->data);
            lval_free(src);
            long got = 0;
            assert(lval_as_num(dest, &got));
            assert(got == 10);
        });
    });

    subdesc(copy, {
        it("copies a LVAL_NUM", {
            long expected = 10;
            struct lval* v = lval_alloc();
            struct lval* cpy = lval_alloc();
            assert(lval_mut_num(v, expected));
            assert(lval_type(v) == LVAL_NUM);
            assert(lval_copy(cpy, v));
            assert(lval_type(cpy) == LVAL_NUM);
            long got = 0;
            assert(lval_as_num(cpy, &got));
            assert(got == expected);
            assert(lval_free(v));
            assert(lval_free(cpy));
        });

        it("copies a LVAL_STR", {
            const char* expected = "expected";
            const char* got = NULL;
            struct lval* v = lval_alloc();
            struct lval* cpy = lval_alloc();
            assert(lval_mut_str(v, expected));
            assert(lval_type(v) == LVAL_STR);
            assert(lval_copy(cpy, v));
            assert(lval_free(v));
            assert(lval_type(cpy) == LVAL_STR);
            assert(got = lval_as_str(cpy));
            assert(strcmp(got, expected) == 0);
            assert(lval_free(cpy));
        });
    });

    subdesc(is_numeric, {
        it("is numeric because it's a LVAL_NIL", {
            struct lval* v = lval_alloc();
            assert(lval_is_numeric(v));
            assert(lval_free(v));
        });

        it("is numeric because it's a LVAL_NUM", {
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, 10));
            assert(lval_is_numeric(v));
            assert(lval_free(v));
        });

        it("is numeric because it's a LVAL_BIGNUM", {
            struct lval* v = lval_alloc();
            mpz_t ibn;
            mpz_init_set_si(ibn, 10);
            assert(lval_mut_bignum(v, ibn));
            mpz_clear(ibn);
            assert(lval_is_numeric(v));
            assert(lval_free(v));
        });

        it("is numeric because it's a LVAL_DBL", {
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, 10.0));
            assert(lval_is_numeric(v));
            assert(lval_free(v));
        });

        it("is not numeric because it's a LVAL_STR", {
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, "not numeric"));
            assert(!lval_is_numeric(v));
            assert(lval_free(v));
        });
    });

    subdesc(is_zero, {
        it("is not equal to 0 because it's a LVAL_NIL", {
            struct lval* v = lval_alloc();
            assert(lval_is_nil(v));
            assert(!lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is equal to 0 because it's a LVAL_NUM equal to 0", {
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, 0));
            assert(lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is not equal to 0 because it's a LVAL_NUM equal to 10", {
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, 10));
            assert(!lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is equal to 0 because it's a LVAL_BIGNUM equal to 0", {
            struct lval* v = lval_alloc();
            mpz_t ibn;
            mpz_init_set_si(ibn, 0);
            assert(lval_mut_bignum(v, ibn));
            mpz_clear(ibn);
            assert(lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is not equal to 0 because it's a LVAL_BIGNUM equal to 10", {
            struct lval* v = lval_alloc();
            mpz_t ibn;
            mpz_init_set_si(ibn, 10);
            assert(lval_mut_bignum(v, ibn));
            mpz_clear(ibn);
            assert(!lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is equal to 0 because it's a LVAL_DBL equal to 0.0", {
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, 0.0));
            assert(lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is not equal to 0 because it's a LVAL_DBL equal to 10.0", {
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, 10.0));
            assert(!lval_is_zero(v));
            assert(lval_free(v));
        });

        it("is not equal to 0 because it's a LVAL_STR", {
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, "not numeric"));
            assert(!lval_is_zero(v));
            assert(lval_free(v));
        });
    });

    subdesc(sign, {
        it("is 0 for LVAL_NIL", {
            struct lval* v = lval_alloc();
            assert(lval_is_nil(v));
            assert(lval_sign(v) == 0);
            assert(lval_free(v));
        });

        it("works for LVAL_NUM", {
            struct {
                long input;
                int  sign;
            } tt[] = {
                { .input=  1, .sign=  1 },
                { .input=  0, .sign=  0 },
                { .input= -1, .sign= -1 },
            };
            struct lval* v = lval_alloc();
            assert(lval_is_nil(v));
            for (int i = 0; i < LENGTH(tt); i++) {
                assert(lval_mut_num(v, tt[i].input));
                int got = lval_sign(v);
                int expected = tt[i].sign;
                if (got != expected) {
                    fail("got %d expected %d", got, expected);
                }
            }
            assert(lval_free(v));
        });

        it("works for LVAL_DBL", {
            struct {
                double input;
                int    sign;
            } tt[] = {
                { .input=  1.0, .sign=  1 },
                { .input=  0.0, .sign=  0 },
                { .input= -1.0, .sign= -1 },
            };
            struct lval* v = lval_alloc();
            assert(lval_is_nil(v));
            for (int i = 0; i < LENGTH(tt); i++) {
                assert(lval_mut_num(v, tt[i].input));
                int got = lval_sign(v);
                int expected = tt[i].sign;
                if (got != expected) {
                    fail("got %d expected %d", got, expected);
                }
            }
            assert(lval_free(v));
        });

        it("works for LVAL_BIGNUM", {
            struct {
                long input;
                int  sign;
            } tt[] = {
                { .input=  1, .sign=  1 },
                { .input=  0, .sign=  0 },
                { .input= -1, .sign= -1 },
            };
            struct lval* v = lval_alloc();
            assert(lval_is_nil(v));
            mpz_t bn; mpz_init(bn);
            for (int i = 0; i < LENGTH(tt); i++) {
                mpz_set_si(bn, tt[i].input);
                assert(lval_mut_bignum(v, bn));
                int got = lval_sign(v);
                int expected = tt[i].sign;
                if (got != expected) {
                    fail("got %d expected %d", got, expected);
                }
            }
            mpz_clear(bn);
            assert(lval_free(v));
        });

    });

    subdesc(are_equal, {
        it("LVAL_NIL are always equal to each other", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            assert(lval_are_equal(a, b));
            assert(lval_mut_num(b, 10));
            assert(!lval_are_equal(a, b));
            assert(lval_clear(b));
            assert(lval_are_equal(a, b));
            assert(lval_free(a));
            assert(lval_free(b));
        });

        it("equal & not equal lvals of type LVAL_NUM", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            assert(lval_mut_num(a, 10));
            assert(lval_mut_num(b, 10));
            assert(lval_are_equal(a, b));
            assert(lval_mut_num(b, 11));
            assert(!lval_are_equal(a, b));
            assert(lval_free(a));
            assert(lval_free(b));
        });

        it("equal & not equal lvals of type LVAL_BIGNUM", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            mpz_t ibn;
            mpz_init(ibn);
            mpz_set_si(ibn, 10);
            assert(lval_mut_bignum(a, ibn));
            assert(lval_mut_bignum(b, ibn));
            mpz_set_si(ibn, 11);
            assert(lval_mut_bignum(b, ibn));
            assert(!lval_are_equal(a, b));
            mpz_clear(ibn);
            assert(lval_free(a));
            assert(lval_free(b));
        });

        it("equal & not equal lvals of type LVAL_DBL", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            assert(lval_mut_dbl(a, .0));
            assert(lval_mut_dbl(b, .0));
            assert(lval_are_equal(a, b));
            assert(lval_mut_dbl(b, 1.0));
            assert(!lval_are_equal(a, b));
            assert(lval_free(a));
            assert(lval_free(b));
        });

        it("equal & not equal lvals of type LVAL_STR", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            assert(lval_mut_str(a, "equal"));
            assert(lval_mut_str(b, "equal"));
            assert(lval_are_equal(a, b));
            assert(lval_mut_str(b, "not equal"));
            assert(!lval_are_equal(a, b));
            assert(lval_free(a));
            assert(lval_free(b));
        });

        it("different types are never equal", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            assert(lval_mut_num(a, 1));
            mpz_t ibn;
            mpz_init_set_si(ibn, 1);
            assert(lval_mut_bignum(b, ibn));
            assert(!lval_are_equal(a, b));
            mpz_clear(ibn);
            assert(lval_mut_dbl(b, 1.0));
            assert(!lval_are_equal(a, b));
            assert(lval_mut_str(b, "not equal"));
            assert(!lval_are_equal(a, b));
            assert(lval_free(a));
            assert(lval_free(b));
        });

    });

    subdesc(compare, {
        it("passes for LVAL_NUM", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            defer(lval_free(a));
            defer(lval_free(b));
            assert(lval_mut_num(a, 10));
            assert(lval_mut_num(b, 20));
            assert(0 > lval_compare(a, b));
            assert(lval_mut_num(a, 20));
            assert(lval_mut_num(b, 20));
            assert(0 == lval_compare(a, b));
            assert(lval_mut_num(a, 20));
            assert(lval_mut_num(b, 10));
            assert(0 < lval_compare(a, b));
        });

        it("passes for LVAL_STR", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            defer(lval_free(a));
            defer(lval_free(b));
            assert(lval_mut_str(a, "za"));
            assert(lval_mut_str(b, "zz"));
            assert(0 > lval_compare(a, b));
            assert(lval_mut_str(a, "zz"));
            assert(lval_mut_str(b, "zz"));
            assert(0 == lval_compare(a, b));
            assert(lval_mut_str(a, "zz"));
            assert(lval_mut_str(b, "za"));
            assert(0 < lval_compare(a, b));
        });

        it("passes for LVAL_QEXPR", {
            struct lval* a = lval_alloc();
            struct lval* b = lval_alloc();
            struct lval* x = lval_alloc();
            struct lval* y = lval_alloc();
            defer(lval_free(a));
            defer(lval_free(b));
            defer(lval_free(x));
            defer(lval_free(y));
            lval_mut_num(x, 10);
            lval_mut_num(y, 20);
            /* a < b */
            assert(lval_mut_qexpr(a));
            lval_push(a, x);
            lval_push(a, x);
            lval_push(b, x);
            lval_push(b, y);
            assert(lval_mut_qexpr(b));
            assert(0 > lval_compare(a, b));
            /* a = b */
            lval_clear(a); lval_clear(b);
            assert(lval_mut_qexpr(a));
            assert(lval_mut_qexpr(b));
            lval_push(a, x);
            lval_push(a, y);
            lval_push(b, x);
            lval_push(b, y);
            assert(0 == lval_compare(a, b));
            /* a > b */
            lval_clear(a); lval_clear(b);
            assert(lval_mut_qexpr(a));
            assert(lval_mut_qexpr(b));
            lval_push(a, x);
            lval_push(a, y);
            lval_push(b, x);
            lval_push(b, x);
            assert(0 < lval_compare(a, b));
        });
    });

    subdesc(sexpr, {
        it("lval_push works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* sexpr = lval_alloc();
            defer(lval_free(sexpr));
            assert(lval_mut_sexpr(sexpr));
            assert(lval_push(sexpr, sym));
            assert(lval_push(sexpr, a));
            assert(lval_push(sexpr, b));
            /* Check. */
            char got[256];
            to_string(sexpr, got);
            assert(strcmp(got, "(+ 1 2)") == 0);
        });

        it("lval_pop works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* sexpr = lval_alloc();
            defer(lval_free(sexpr));
            assert(lval_mut_sexpr(sexpr));
            assert(lval_push(sexpr, sym));
            assert(lval_push(sexpr, a));
            assert(lval_push(sexpr, b));
            struct lval* val;
            assert(val = lval_pop(sexpr, 1));
            defer(lval_free(val));
            /* Check. */
            char got[256];
            to_string(sexpr, got);
            assert(strcmp(got, "(+ 2)") == 0);
            assert(lval_are_equal(val, a));
        });

        it("lval_index works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* sexpr = lval_alloc();
            defer(lval_free(sexpr));
            assert(lval_mut_sexpr(sexpr));
            assert(lval_push(sexpr, sym));
            assert(lval_push(sexpr, a));
            assert(lval_push(sexpr, b));
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lval_index(sexpr, 1, got));
            assert(lval_are_equal(got, a));
        });

        it("lval_len works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_str(a, "1");
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_str(b, "2");
            defer(lval_free(b));
            struct lval* sexpr = lval_alloc();
            defer(lval_free(sexpr));
            assert(lval_mut_sexpr(sexpr));
            assert(lval_push(sexpr, sym));
            assert(lval_push(sexpr, a));
            assert(lval_push(sexpr, b));
            assert(lval_len(sexpr) == 3);
        });
    });

    subdesc(qexpr, {
        it("lval_cons works", {
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* c = lval_alloc();
            lval_mut_num(c, 3);
            defer(lval_free(c));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_cons(qexpr, c));
            assert(lval_cons(qexpr, b));
            assert(lval_cons(qexpr, a));
            /* Check. */
            char got[256];
            to_string(qexpr, got);
            assert(strcmp(got, "{1 2 3}") == 0);
        });

        it("lval_push works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_push(qexpr, sym));
            assert(lval_push(qexpr, a));
            assert(lval_push(qexpr, b));
            /* Check. */
            char got[256];
            to_string(qexpr, got);
            assert(strcmp(got, "{+ 1 2}") == 0);
        });

        it("lval_pop works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_push(qexpr, sym));
            assert(lval_push(qexpr, a));
            assert(lval_push(qexpr, b));
            struct lval* val;
            assert(val = lval_pop(qexpr, 1));
            defer(lval_free(val));
            /* Check. */
            char got[256];
            to_string(qexpr, got);
            assert(strcmp(got, "{+ 2}") == 0);
            assert(lval_are_equal(val, a));
        });

        it("lval_index works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_push(qexpr, sym));
            assert(lval_push(qexpr, a));
            assert(lval_push(qexpr, b));
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lval_index(qexpr, 1, got));
            assert(lval_are_equal(got, a));
        });

        it("lval_len works", {
            struct lval* sym = lval_alloc();
            lval_mut_sym(sym, "+");
            defer(lval_free(sym));
            struct lval* a = lval_alloc();
            lval_mut_str(a, "1");
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_str(b, "2");
            defer(lval_free(b));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_push(qexpr, sym));
            assert(lval_push(qexpr, a));
            assert(lval_push(qexpr, b));
            assert(lval_len(qexpr) == 3);
        });
    });

    subdesc(str, {
        it("lval_len works", {
            const char* input = "init";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            assert(lval_len(a) == strlen(input));
        });

        it("lval_cons works", {
            const char* input = "init";
            const char* elem = "...";
            const char* expected = "...init";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            lval_mut_str(b, elem);
            /* Cons. */
            assert(lval_cons(a, b));
            /* Test. */
            assert(lval_len(a) == strlen(expected));
            assert(strcmp(expected, lval_as_str(a)) == 0);
        });

        it("lval_push works", {
            const char* input = "init";
            const char* elem = "...";
            const char* expected = "init...";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            lval_mut_str(b, elem);
            /* Cons. */
            assert(lval_push(a, b));
            /* Test. */
            assert(lval_len(a) == strlen(expected));
            assert(strcmp(expected, lval_as_str(a)) == 0);
        });

        it("lval_pop works", {
            const char* input = "init";
            const char* expected_a = "int";
            const char* expected_b = "i";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            /* Cons. */
            struct lval* b = NULL;
            assert(b = lval_pop(a, 2));
            defer(if (b) lval_free(b));
            /* Test. */
            assert(strcmp(expected_b, lval_as_str(b)) == 0);
            assert(lval_len(a) == strlen(expected_a));
            assert(strcmp(expected_a, lval_as_str(a)) == 0);
        });

        it("lval_drop works for the last element", {
            const char* input = "i";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            /* Cons. */
            lval_drop(a, 0);
            /* Test. */
            assert(lval_len(a) == 0);
            assert(lval_as_str(a) == NULL);
        });

        it("lval_index works", {
            const char* input = "init";
            const char* expected_a = "init";
            const char* expected_b = "i";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Cons. */
            assert(lval_index(a, 2, b));
            /* Test. */
            assert(strcmp(expected_b, lval_as_str(b)) == 0);
            assert(lval_len(a) == strlen(expected_a));
            assert(strcmp(expected_a, lval_as_str(a)) == 0);
        });
    });

    subdesc(copy_range, {
        it("passes for Q-Expressing", {
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            assert(lval_mut_qexpr(a));
            push_num(a, 1);
            push_num(a, 2);
            push_num(a, 3);
            push_num(a, 4);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Copy. */
            assert(lval_copy_range(b, a, 1, 3));
            /* Test. */
            struct lval* expected = lval_alloc();
            defer(lval_free(expected));
            lval_mut_qexpr(expected);
            push_num(expected, 2);
            push_num(expected, 3);
            assert(lval_are_equal(expected, b));
        });
        it("passes for strings", {
            const char* input = "alongstring";
            const char* expected = "long";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Copy. */
            assert(lval_copy_range(b, a, 1, 5));
            /* Test. */
            assert(strcmp(expected, lval_as_str(b)) == 0);
        });
        it("passes for strings; first > len", {
            const char* input = "alongstring";
            const char* expected = "";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Copy. */
            assert(lval_copy_range(b, a, 100, 5));
            /* Test. */
            assert(strcmp(expected, lval_as_str(b)) == 0);
        });
        it("passes for strings; last > len", {
            const char* input = "alongstring";
            const char* expected = input;
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Copy. */
            assert(lval_copy_range(b, a, 0, strlen(input) + 100));
            /* Test. */
            assert(strcmp(expected, lval_as_str(b)) == 0);
        });
    });

    subdesc(reverse, {
        it("passes for Q-Expressing", {
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            assert(lval_mut_qexpr(a));
            push_num(a, 1);
            push_num(a, 2);
            push_num(a, 3);
            push_num(a, 4);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Reverse. */
            assert(lval_reverse(b, a));
            /* Test. */
            struct lval* expected = lval_alloc();
            defer(lval_free(expected));
            lval_mut_qexpr(expected);
            push_num(expected, 4);
            push_num(expected, 3);
            push_num(expected, 2);
            push_num(expected, 1);
            assert(lval_are_equal(expected, b));
        });
        it("passes for empty Q-Expressing", {
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            assert(lval_mut_qexpr(a));
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Reverse. */
            assert(lval_reverse(b, a));
            /* Test. */
            struct lval* expected = lval_alloc();
            defer(lval_free(expected));
            lval_mut_qexpr(expected);
            assert(lval_are_equal(expected, b));
        });
        it("passes for strings", {
            const char* input = "input";
            const char* expected = "tupni";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Reverse. */
            assert(lval_reverse(b, a));
            /* Test. */
            assert(strcmp(expected, lval_as_str(b)) == 0);
        });
        it("passes for empty string", {
            const char* input = "";
            const char* expected = "";
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            lval_mut_str(a, input);
            struct lval* b = lval_alloc();
            defer(lval_free(b));
            /* Reverse. */
            assert(lval_reverse(b, a));
            /* Test. */
            assert(strcmp(expected, lval_as_str(b)) == 0);
        });
    });

    subdesc(swap, {
        it("passes for Q-Expressing", {
            struct lval* a = lval_alloc();
            defer(lval_free(a));
            assert(lval_mut_qexpr(a));
            push_num(a, 1);
            push_num(a, 2);
            push_num(a, 3);
            push_num(a, 4);
            /* Swap. */
            assert(lval_swap(a, 0, 3));
            /* Test. */
            struct lval* expected = lval_alloc();
            defer(lval_free(expected));
            lval_mut_qexpr(expected);
            push_num(expected, 4);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 1);
            assert(lval_are_equal(expected, a));
        });
    });

    subdesc(print_to, {
        it("prints a num", {
            long input = 10;
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, input));
            assert(lval_type(v) == LVAL_NUM);
            defer(lval_free(v));
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "10") == 0);
        });

        it("prints a bignum", {
            mpz_t input; /** = 10^20 */
            mpz_init_set_si(input, 10);
            mpz_pow_ui(input, input, 20u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, input));
            assert(lval_type(v) == LVAL_BIGNUM);
            defer(lval_free(v));
            mpz_clear(input);
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "100000000000000000000") == 0);
        });

        it("prints a double", {
            double input = 0.42;
            struct lval* v = lval_alloc();
            assert(lval_mut_dbl(v, input));
            assert(lval_type(v) == LVAL_DBL);
            defer(lval_free(v));
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "0.42") == 0);
        });

        it("prints an error", {
            enum lerr_code input = LERR_EVAL;
            struct lval* v = lval_alloc();
            assert(lval_mut_err_code(v, input));
            struct lerr* err = lval_as_err(v);
            lerr_annotate(err, "error");
            assert(lval_type(v) == LVAL_ERR);
            defer(lval_free(v));
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "Error: error") == 0);
        });

        it("prints a string", {
            const char* input = "string";
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, input));
            assert(lval_type(v) == LVAL_STR);
            defer(lval_free(v));
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "\"string\"") == 0);
        });

        it("prints a symbol", {
            const char* input = "symbol";
            struct lval* v = lval_alloc();
            assert(lval_mut_sym(v, input));
            assert(lval_type(v) == LVAL_SYM);
            defer(lval_free(v));
            char got[256];
            to_string(v, got);
            assert(strcmp(got, "symbol") == 0);
        });

        it("prints a S-Expression", {
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* c = lval_alloc();
            lval_mut_num(c, 3);
            defer(lval_free(c));
            struct lval* sexpr = lval_alloc();
            defer(lval_free(sexpr));
            assert(lval_mut_sexpr(sexpr));
            assert(lval_push(sexpr, a));
            assert(lval_push(sexpr, b));
            assert(lval_push(sexpr, c));
            char got[256];
            to_string(sexpr, got);
            assert(strcmp(got, "(1 2 3)") == 0);
        });

        it("prints a Q-Expression", {
            struct lval* a = lval_alloc();
            lval_mut_num(a, 1);
            defer(lval_free(a));
            struct lval* b = lval_alloc();
            lval_mut_num(b, 2);
            defer(lval_free(b));
            struct lval* c = lval_alloc();
            lval_mut_num(c, 3);
            defer(lval_free(c));
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            assert(lval_mut_qexpr(qexpr));
            assert(lval_push(qexpr, a));
            assert(lval_push(qexpr, b));
            assert(lval_push(qexpr, c));
            char got[256];
            to_string(qexpr, got);
            assert(strcmp(got, "{1 2 3}") == 0);
        });
    });

});

snow_main();
