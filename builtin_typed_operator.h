#ifndef _H_OPERATOR_
#define _H_OPERATOR_

#include <limits.h>
#include <math.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "lval.h"

/* Macros */
#define UNUSED(x) (void)(x)

/* Conditions: typed conditions for long. */
inline bool cnd_num_add_overflow(const long a, const long b) {
    return a >= LONG_MAX - b;
}

inline bool cnd_num_sub_overflow(const long a, const long b) {
    return a <= LONG_MIN + b;
}

inline bool cnd_num_mul_overflow(const long a, const long b) {
    return ((b > 0 && ((a > 0 && a > LONG_MAX / b) || (a < 0 && a < LONG_MIN / b))) ||
            (b == -1 && a == -LONG_MAX) ||
            (b < -1 && ((a < 0 && a < LONG_MAX / b) || (a > 0 && a > LONG_MIN / b))));
}
inline bool cnd_num_pow_overflow(const long a, const long b) {
    long base = (a < 0) ? -a : a;
    return (double)(b) > log2((double)LONG_MAX)/log2((double)base);
}

inline bool cnd_num_fact_overflow(const long a, const long b) {
    UNUSED(b);
    return a > 20;
}

/* Operators: long. */
inline long op_num_nop(const long a, const long b) {
    UNUSED(a);
    UNUSED(b);
    return 0;
}

inline long op_num_add(const long a, const long b) {
    return a + b;
}

inline long op_num_sub(const long a, const long b) {
    return a - b;
}

inline long op_num_mul(const long a, const long b) {
    return a * b;
}

inline long op_num_div(const long a, const long b) {
    return a / b;
}

inline long op_num_mod(const long a, const long b) {
    return a % b;
}

long op_num_pow(const long a, const long b);
long op_num_fact(const long a, const long b);

/* Operators: double. */
inline double op_dbl_nop(const double a, const double b) {
    UNUSED(a);
    UNUSED(b);
    return NAN;
}

inline double op_dbl_add(const double a, const double b) {
    return a + b;
}

inline double op_dbl_sub(const double a, const double b) {
    return a - b;
}

inline double op_dbl_mul(const double a, const double b) {
    return a * b;
}

inline double op_dbl_div(const double a, const double b) {
    return a / b;
}

inline double op_dbl_pow(const double a, const double b) {
    return pow(a, b);
}

/* Operators: bignum. */
void op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b);
void op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b);

#endif
