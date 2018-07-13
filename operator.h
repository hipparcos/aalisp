#ifndef _H_OPERATOR_
#define _H_OPERATOR_

#include <limits.h>
#include <math.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "lval.h"

/* Macros */
#define UNUSED(x) (void)(x)

/* Condition functions. */
inline bool cnd_y_is_zero(struct lval x, struct lval y) {
    UNUSED(x);
    return lval_is_zero(y);
}

inline bool cnd_either_is_dbl(struct lval x, struct lval y) {
    return x.type == LVAL_DBL || y.type == LVAL_DBL;
}

inline bool cnd_either_is_bignum(struct lval x, struct lval y) {
    return x.type == LVAL_BIGNUM || y.type == LVAL_BIGNUM;
}

inline bool cnd_dbl_and_bignum(struct lval x, struct lval y) {
    return cnd_either_is_dbl(x,y) && cnd_either_is_bignum(x,y);
}

inline bool cnd_are_num(struct lval x, struct lval y) {
    return x.type == LVAL_NUM && (y.type == LVAL_NUM || y.type == LVAL_NIL);
}

inline bool cnd_x_is_neg(struct lval x, struct lval y) {
    UNUSED(y);
    return (x.type == LVAL_NUM && x.data.num < 0) ||
           (x.type == LVAL_BIGNUM && mpz_sgn(x.data.bignum) < 0) ||
           (x.type == LVAL_DBL && x.data.dbl < 0);
}

inline bool cnd_x_too_big_for_ul(struct lval x, struct lval y) {
    UNUSED(y);
    return x.type == LVAL_BIGNUM && mpz_cmp_ui(x.data.bignum, ULONG_MAX) > 0;
}

/* Conditions for long. */
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

/* Operators: bignum. */
void op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b);

#endif
