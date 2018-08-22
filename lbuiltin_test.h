#include "lbuiltin.h"

#include "vendor/snow/snow/snow.h"

#include "lval.h"
#include "lerr.h"
#include "lenv.h"
#include "lfunc.h"

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

#define push_func(args, fun) \
    do { \
        struct lval* x = lval_alloc(); \
        lval_mut_func(x, fun); \
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
