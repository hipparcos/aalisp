#include "lbuiltin_operator.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "vendor/mini-gmp/mini-gmp.h"

/* Macros */
#define UNUSED(x) (void)(x)

/* Conditions: overflow conditions for long. */
bool lbi_cond_num_add_overflow(const long a, const long b) {
    return a >= LONG_MAX - b;
}

bool lbi_cond_num_sub_overflow(const long a, const long b) {
    return a <= LONG_MIN + b;
}

bool lbi_cond_num_mul_overflow(const long a, const long b) {
    return ((b > 0 && ((a > 0 && a > LONG_MAX / b) || (a < 0 && a < LONG_MIN / b))) ||
            (b == -1 && a == -LONG_MAX) ||
            (b < -1 && ((a < 0 && a < LONG_MAX / b) || (a > 0 && a > LONG_MIN / b))));
}
bool lbi_cond_num_pow_overflow(const long a, const long b) {
    long base = labs(a);
    return (double)(b) > log2((double)LONG_MAX)/log2((double)base);
}

bool lbi_cond_num_fac_overflow(const long a, const long b) {
    UNUSED(a);
    return b > 20;
}

/* Operators: long. */
long lbi_op_num_add(const long a, const long b) {
    return a + b;
}

long lbi_op_num_sub(const long a, const long b) {
    return a - b;
}

long lbi_op_num_sub_unary(const long a, const long b) {
    UNUSED(b);
    return -a;
}

long lbi_op_num_mul(const long a, const long b) {
    return a * b;
}

long lbi_op_num_div(const long a, const long b) {
    return a / b;
}

long lbi_op_num_mod(const long a, const long b) {
    return a % b;
}

long lbi_op_num_pow(const long a, const long b) {
    long x = a;
    for (int n = b; n > 1; n--) {
        x *= a;
    }
    return x;
}

long lbi_op_num_fac(const long a, const long b) {
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
double lbi_op_dbl_nop(const double a, const double b) {
    UNUSED(a);
    UNUSED(b);
    return NAN;
}

double lbi_op_dbl_add(const double a, const double b) {
    return a + b;
}

double lbi_op_dbl_sub(const double a, const double b) {
    return a - b;
}

double lbi_op_dbl_sub_unary(const double a, const double b) {
    UNUSED(b);
    return -a;
}

double lbi_op_dbl_mul(const double a, const double b) {
    return a * b;
}

double lbi_op_dbl_div(const double a, const double b) {
    return a / b;
}

double lbi_op_dbl_pow(const double a, const double b) {
    return pow(a, b);
}

/* Operators: bignum. */
void lbi_op_bignum_sub_unary(mpz_t r, const mpz_t a, const mpz_t b) {
    UNUSED(b);
    mpz_ui_sub(r, 0, a);
}

void lbi_op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b) {
    UNUSED(a);
    unsigned long n = mpz_get_ui(b);
    if (n == 0 || n == 1) {
        mpz_set_si(r, 1);
    }
    mpz_fac_ui(r, n);
}

void lbi_op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b) {
    unsigned long n = mpz_get_ui(b);
    mpz_pow_ui(r, a, n);
}
