#include "lbuiltin_operator.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "vendor/mini-gmp/mini-gmp.h"

#include "lval.h"
#include "lenv.h"

#define UNUSED(x) (void)(x)

/* Conditions: overflow conditions for long. */
static bool lbi_cond_num_add_overflow(const long a, const long b) {
    return a >= LONG_MAX - b;
}

static bool lbi_cond_num_sub_overflow(const long a, const long b) {
    return a <= LONG_MIN + b;
}

static bool lbi_cond_num_mul_overflow(const long a, const long b) {
    return ((b > 0 && ((a > 0 && a > LONG_MAX / b) || (a < 0 && a < LONG_MIN / b))) ||
            (b == -1 && a == -LONG_MAX) ||
            (b < -1 && ((a < 0 && a < LONG_MAX / b) || (a > 0 && a > LONG_MIN / b))));
}
static bool lbi_cond_num_pow_overflow(const long a, const long b) {
    long base = labs(a);
    return (double)(b) > log2((double)LONG_MAX)/log2((double)base);
}

static bool lbi_cond_num_fac_overflow(const long a, const long b) {
    UNUSED(a);
    return b > 20;
}

/* Operators: long. */
static long lbi_op_num_add(const long a, const long b) {
    return a + b;
}

static long lbi_op_num_sub(const long a, const long b) {
    return a - b;
}

static long lbi_op_num_mul(const long a, const long b) {
    return a * b;
}

static long lbi_op_num_div(const long a, const long b) {
    return a / b;
}

static long lbi_op_num_mod(const long a, const long b) {
    return a % b;
}

static long lbi_op_num_pow(const long a, const long b) {
    long x = a;
    for (int n = b; n > 1; n--) {
        x *= a;
    }
    return x;
}

static long lbi_op_num_fac(const long a, const long b) {
    UNUSED(a);
    if (b == 0) {
        return 1;
    }
    long fact = b;
    long n    = b;
    while (n > 2) {
        fact *= --n;
    }
    return fact;
}

/* Operators: double. */
static double lbi_op_dbl_add(const double a, const double b) {
    return a + b;
}

static double lbi_op_dbl_sub(const double a, const double b) {
    return a - b;
}

static double lbi_op_dbl_mul(const double a, const double b) {
    return a * b;
}

static double lbi_op_dbl_div(const double a, const double b) {
    return a / b;
}

static double lbi_op_dbl_pow(const double a, const double b) {
    return pow(a, b);
}

/* Operators: bignum. */
static void lbi_op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b) {
    UNUSED(a);
    unsigned long n = mpz_get_ui(b);
    if (n == 0 || n == 1) {
        mpz_set_si(r, 1);
    }
    mpz_fac_ui(r, n);
}

static void lbi_op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b) {
    unsigned long n = mpz_get_ui(b);
    mpz_pow_ui(r, a, n);
}

#define EITHER_IS(type, a, b) (lval_type(a) == type || lval_type(b) == type)
static enum ltype typeof_op(const struct lval* a, const struct lval *b) {
    if (EITHER_IS(LVAL_DBL, a, b))    return LVAL_DBL;
    if (EITHER_IS(LVAL_BIGNUM, a, b)) return LVAL_BIGNUM;
    if (EITHER_IS(LVAL_NUM, a, b))    return LVAL_NUM;
    return LVAL_NIL;
}

/** lbuiltin_operator does the automatic casting of args then execute op. */
static int lbuiltin_operator(
        /** op_* operates on basic types: dbl > bignum > num. */
        long   (*op_num)(const long, const long),
        void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y),
        double (*op_dbl)(const double, const double),
        /** cnd_overflow tells if an operation on nums must be casted to bignum. */
        bool   (*cnd_overflow)(const long, const long),
        /* lbuitin arguments. */
        struct lenv* env, const struct lval* arg, struct lval* acc) {
    switch (typeof_op(acc, arg)) {
    case LVAL_DBL:
        {
        if (!op_dbl) {
            return -1;
        }
        double a, b;
        lval_as_dbl(acc, &a);
        lval_as_dbl(arg, &b);
        lval_mut_dbl(acc, op_dbl(a, b));
        return 0;
        }
    case LVAL_BIGNUM:
        {
        mpz_t a, b;
        mpz_init(a);
        mpz_init(b);
        lval_as_bignum(acc, a);
        lval_as_bignum(arg, b);
        mpz_t rbn;
        mpz_init(rbn);
        op_bignum(rbn, a, b);
        mpz_clear(a);
        mpz_clear(b);
        lval_mut_bignum(acc, rbn);
        mpz_clear(rbn);
        return 0;
        }
    case LVAL_NUM:
        {
        long a, b;
        lval_as_num(acc, &a);
        lval_as_num(arg, &b);
        if (cnd_overflow && cnd_overflow(a, b)) {
            mpz_t bna;
            mpz_init_set_si(bna, a);
            lval_mut_bignum(acc, bna);
            int s = lbuiltin_operator(
                    op_num, op_bignum, op_dbl, cnd_overflow,
                    env, arg, acc);
            mpz_clear(bna);
            return s;
        }
        lval_mut_num(acc, op_num(a, b));
        return 0;
        }
    default: break;
    }

    lval_mut_err(acc, LERR_EVAL);
    return -1;
}

/** Exported operators. */
int lbi_op_add(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_add,
            mpz_add,
            lbi_op_dbl_add,
            lbi_cond_num_add_overflow,
            env, arg, acc);
}

int lbi_op_sub(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_sub,
            mpz_sub,
            lbi_op_dbl_sub,
            lbi_cond_num_sub_overflow,
            env, arg, acc);
}

int lbi_op_mul(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_mul,
            mpz_mul,
            lbi_op_dbl_mul,
            lbi_cond_num_mul_overflow,
            env, arg, acc);
}

int lbi_op_div(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_div,
            mpz_fdiv_q,
            lbi_op_dbl_div,
            NULL,
            env, arg, acc);
}

int lbi_op_mod(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_mod,
            mpz_mod,
            NULL,
            NULL,
            env, arg, acc);
}

int lbi_op_fac(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_fac,
            lbi_op_bignum_fac,
            NULL,
            lbi_cond_num_fac_overflow,
            env, arg, acc);
}

int lbi_op_pow(struct lenv* env, const struct lval* arg, struct lval* acc) {
    return lbuiltin_operator(
            lbi_op_num_pow,
            lbi_op_bignum_pow,
            lbi_op_dbl_pow,
            lbi_cond_num_pow_overflow,
            env, arg, acc);
}
