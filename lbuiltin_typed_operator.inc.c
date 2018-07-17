#include <limits.h>
#include <math.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "lval.h"

/* Macros */
#define UNUSED(x) (void)(x)

/* Conditions: typed conditions for long. */
inline static bool cnd_num_add_overflow(const long a, const long b) {
    return a >= LONG_MAX - b;
}

inline static bool cnd_num_sub_overflow(const long a, const long b) {
    return a <= LONG_MIN + b;
}

inline static bool cnd_num_mul_overflow(const long a, const long b) {
    return ((b > 0 && ((a > 0 && a > LONG_MAX / b) || (a < 0 && a < LONG_MIN / b))) ||
            (b == -1 && a == -LONG_MAX) ||
            (b < -1 && ((a < 0 && a < LONG_MAX / b) || (a > 0 && a > LONG_MIN / b))));
}
inline static bool cnd_num_pow_overflow(const long a, const long b) {
    long base = (a < 0) ? -a : a;
    return (double)(b) > log2((double)LONG_MAX)/log2((double)base);
}

inline static bool cnd_num_fact_overflow(const long a, const long b) {
    UNUSED(b);
    return a > 20;
}

/* Operators: long. */
inline static long op_num_nop(const long a, const long b) {
    UNUSED(a);
    UNUSED(b);
    return 0;
}

inline static long op_num_add(const long a, const long b) {
    return a + b;
}

inline static long op_num_sub(const long a, const long b) {
    return a - b;
}

inline static long op_num_mul(const long a, const long b) {
    return a * b;
}

inline static long op_num_div(const long a, const long b) {
    return a / b;
}

inline static long op_num_mod(const long a, const long b) {
    return a % b;
}

static long op_num_pow(const long a, const long b);
static long op_num_fact(const long a, const long b);

/* Operators: double. */
inline static double op_dbl_nop(const double a, const double b) {
    UNUSED(a);
    UNUSED(b);
    return NAN;
}

inline static double op_dbl_add(const double a, const double b) {
    return a + b;
}

inline static double op_dbl_sub(const double a, const double b) {
    return a - b;
}

inline static double op_dbl_mul(const double a, const double b) {
    return a * b;
}

inline static double op_dbl_div(const double a, const double b) {
    return a / b;
}

inline static double op_dbl_pow(const double a, const double b) {
    return pow(a, b);
}

/* Operators: bignum. */
static void op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b);
static void op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b);

/* Conditions (inline) */
static bool cnd_num_add_overflow(const long a, const long b);
static bool cnd_num_sub_overflow(const long a, const long b);
static bool cnd_num_mul_overflow(const long a, const long b);
static bool cnd_num_fact_overflow(const long a, const long b);
static bool cnd_num_pow_overflow(const long a, const long b);

/* Operators: long. */
/* For inline definitions -> .h */
static long op_num_nop(const long a, const long b);
static long op_num_add(const long a, const long b);
static long op_num_sub(const long a, const long b);
static long op_num_mul(const long a, const long b);
static long op_num_div(const long a, const long b);
static long op_num_mod(const long a, const long b);

static long op_num_pow(const long a, const long b) {
    long x = a;
    for (int n = b; n > 1; n--) {
        x *= a;
    }
    return x;
}

static long op_num_fact(const long a, const long b) {
    UNUSED(b);
    if (a == 0) {
        return 1;
    }
    long fact = a;
    long n    = a;
    while (n > 2) {
        fact *= --n;
    }
    return fact;
}

/* Operators: double. */
/* For inline definitions -> .h */
static double op_dbl_nop(const double a, const double b);
static double op_dbl_add(const double a, const double b);
static double op_dbl_sub(const double a, const double b);
static double op_dbl_mul(const double a, const double b);
static double op_dbl_div(const double a, const double b);
static double op_dbl_pow(const double a, const double b);

/* Operators: bignum. */
static void op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b) {
    UNUSED(b);
    unsigned long n = mpz_get_ui(a);
    if (n == 0 || n == 1) {
        mpz_set_si(r, 1);
    }
    mpz_fac_ui(r, n);
}

static void op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b) {
    unsigned long n = mpz_get_ui(b);
    mpz_pow_ui(r, a, n);
}
