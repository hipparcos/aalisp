#include "lval.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "vendor/snow/snow/snow.h"

#define LENGTH(array) sizeof(array)/sizeof(array[0])

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
            assert(lval_mut_err(v, input));
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

        it("mutates a lval to a num", {
            long input = 10;
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, input));
            assert(lval_type(v) == LVAL_NUM);
            assert(lval_free(v));
        });

        it("mutates a lval to a bignum", {
            mpz_t input; /** = 10^20 */
            mpz_init_set_si(input, 10);
            mpz_pow_ui(input, input, 20u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, input));
            assert(lval_type(v) == LVAL_BIGNUM);
            assert(lval_free(v));
            mpz_clear(input);
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
            assert(lval_mut_err(v, expected));
            assert(lval_type(v) == LVAL_ERR);
            assert(lval_as_err(v, &got));
            assert(got == expected);
            assert(lval_free(v));
        });

        it("mutates a lval to a string and read it back", {
            const char* expected = "expected";
            size_t len = 0;
            struct lval* v = lval_alloc();
            assert(lval_mut_str(v, expected));
            assert(lval_type(v) == LVAL_STR);
            assert(len = lval_printlen(v));
            char* got = calloc(sizeof(char), len);
            assert(lval_as_str(v, got, len));
            assert(strcmp(got, expected) == 0);
            free(got);
            assert(lval_free(v));
        });

        it("mutates a lval to a symbol and read it back", {
            const char* expected = "expected";
            const char* got;
            struct lval* v = lval_alloc();
            assert(lval_mut_sym(v, expected));
            assert(lval_type(v) == LVAL_SYM);
            assert(got = lval_as_sym(v));
            assert(strcmp(got, expected) == 0);
            assert(lval_free(v));
        });

        it("mutates a lval to a num and read it back as a string", {
            long input = 10;
            const char* expected = "10";
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, input));
            assert(lval_type(v) == LVAL_NUM);
            size_t len = 0;
            assert(len = lval_printlen(v));
            char* got = calloc(sizeof(char), len);
            assert(lval_as_str(v, got, len));
            assert(strcmp(got, expected) == 0);
            free(got);
            assert(lval_free(v));
        });

        it("mutates a lval to a bignum and read it back as a string", {
            const char* expected = "100000000000000000000";
            mpz_t input; /** = 10^20 */
            mpz_init_set_si(input, 10);
            mpz_pow_ui(input, input, 20u);
            struct lval* v = lval_alloc();
            assert(lval_mut_bignum(v, input));
            assert(lval_type(v) == LVAL_BIGNUM);
            size_t len = 0;
            assert(len = lval_printlen(v));
            char* got = calloc(sizeof(char), len);
            assert(lval_as_str(v, got, len));
            assert(strcmp(got, expected) == 0);
            free(got);
            assert(lval_free(v));
            mpz_clear(input);
        });

        it("mutates then reads back the same lval to a num, a bignum and finally to a str (no leakages)", {
            long inum = 10;
            mpz_t ibn; /** = 10^20 */
            mpz_init_set_si(ibn, 10);
            mpz_pow_ui(ibn, ibn, 20u);
            const char* istr = "input";
            struct lval* v = lval_alloc();
            assert(lval_mut_num(v, inum));
            assert(lval_type(v) == LVAL_NUM);
            long gl = 0;
            assert(lval_as_num(v, &gl));
            assert(gl = inum);
            assert(lval_mut_bignum(v, ibn));
            assert(lval_type(v) == LVAL_BIGNUM);
            mpz_t gbn;
            mpz_init(gbn);
            assert(lval_as_bignum(v, gbn));
            assert(mpz_cmp(gbn, ibn) == 0 && gbn != ibn);
            mpz_clear(gbn);
            assert(lval_mut_str(v, istr));
            assert(lval_type(v) == LVAL_STR);
            size_t len = 0;
            assert(len = lval_printlen(v));
            char* gstr = calloc(sizeof(char), len);
            assert(lval_as_str(v, gstr, len));
            assert(strcmp(gstr, istr) == 0 && gstr != istr);
            free(gstr);
            assert(lval_free(v));
            mpz_clear(ibn);
        });

        it("works for nested s-expr (+ 1 (+ 1)) as string", {
            const char* expected = "(+ 1 (+ 1))";
            /* Input lval construction. */
            struct lval *input, *sexpr2, *sym, *opr;
            input = lval_alloc();
            sexpr2 = lval_alloc();
            sym = lval_alloc();
            opr = lval_alloc();
            defer(lval_free(opr));
            defer(lval_free(sym));
            defer(lval_free(sexpr2));
            defer(lval_free(input));
            lval_mut_num(opr, 1);
            lval_mut_sym(sym, "+");
            lval_mut_sexpr(sexpr2);
            lval_mut_sexpr(input);
            lval_push(sexpr2, sym);
            lval_push(sexpr2, opr);
            lval_push(input, sym);
            lval_push(input, opr);
            lval_push(input, sexpr2);
            /* Test */
            size_t len = lval_printlen(input);
            assert(len-1 == strlen(expected)); // - '\0'.
            char* got = malloc(len);
            defer(free(got));
            lval_as_str(input, got, len);
            assert(strcmp(got, expected) == 0);
        });

        it("works for nested q-expr {1 2 {3 4}} as string", {
            const char* expected = "{1 1 {1 1}}";
            /* Input lval construction. */
            struct lval *qexpr1, *qexpr2, *opr;
            qexpr1 = lval_alloc();
            qexpr2 = lval_alloc();
            opr = lval_alloc();
            defer(lval_free(opr));
            defer(lval_free(qexpr2));
            defer(lval_free(qexpr1));
            lval_mut_num(opr, 1);
            lval_mut_qexpr(qexpr2);
            lval_mut_qexpr(qexpr1);
            lval_push(qexpr2, opr);
            lval_push(qexpr2, opr);
            lval_push(qexpr1, opr);
            lval_push(qexpr1, opr);
            lval_push(qexpr1, qexpr2);
            /* Test */
            size_t len = lval_printlen(qexpr1);
            assert(len-1 == strlen(expected)); // - '\0'.
            char* got = malloc(len);
            defer(free(got));
            lval_as_str(qexpr1, got, len);
            assert(strcmp(got, expected) == 0);
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
            struct lval* v = lval_alloc();
            struct lval* cpy = lval_alloc();
            assert(lval_mut_str(v, expected));
            assert(lval_type(v) == LVAL_STR);
            assert(lval_copy(cpy, v));
            assert(lval_type(cpy) == LVAL_STR);
            size_t len = 0;
            assert(len = lval_printlen(v));
            char* got = calloc(sizeof(char), len);
            assert(lval_as_str(cpy, got, len));
            assert(strcmp(got, expected) == 0);
            assert(got != expected); // Not the same pointed data.
            free(got);
            assert(lval_free(v));
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
            size_t len = lval_printlen(sexpr);
            char* s = calloc(len, sizeof(char));
            lval_as_str(sexpr, s, len);
            defer(free(s));
            assert(strcmp(s, "(+ 1 2)") == 0);
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
            size_t len = lval_printlen(val);
            char* s = calloc(len, sizeof(char));
            lval_as_str(val, s, len);
            defer(free(s));
            assert(strcmp(s, "1") == 0);
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
            size_t len = lval_printlen(got);
            char* s = calloc(len, sizeof(char));
            lval_as_str(got, s, len);
            defer(free(s));
            assert(strcmp(s, "1") == 0);
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
            size_t len = lval_printlen(qexpr);
            char* s = calloc(len, sizeof(char));
            lval_as_str(qexpr, s, len);
            fputs(s,stdout);
            defer(free(s));
            assert(strcmp(s, "{1 2 3}") == 0);
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
            size_t len = lval_printlen(qexpr);
            char* s = calloc(len, sizeof(char));
            lval_as_str(qexpr, s, len);
            defer(free(s));
            assert(strcmp(s, "{+ 1 2}") == 0);
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
            size_t len = lval_printlen(val);
            char* s = calloc(len, sizeof(char));
            lval_as_str(val, s, len);
            defer(free(s));
            assert(strcmp(s, "1") == 0);
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
            size_t len = lval_printlen(got);
            char* s = calloc(len, sizeof(char));
            lval_as_str(got, s, len);
            defer(free(s));
            assert(strcmp(s, "1") == 0);
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

});

snow_main();
