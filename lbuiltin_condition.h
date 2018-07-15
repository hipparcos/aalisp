#ifndef _H_BUILTIN_CONDITION_
#define _H_BUILTIN_CONDITION_

#include <limits.h>

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

inline bool _is_neg(struct lval x) {
    return (x.type == LVAL_NUM && x.data.num < 0) ||
           (x.type == LVAL_BIGNUM && mpz_sgn(x.data.bignum) < 0) ||
           (x.type == LVAL_DBL && x.data.dbl < 0);
}
inline bool cnd_x_is_neg(struct lval x, struct lval y) {
    UNUSED(y);
    return _is_neg(x);
}
inline bool cnd_y_is_neg(struct lval x, struct lval y) {
    UNUSED(x);
    return _is_neg(y);
}

inline bool _too_big_for_ul(struct lval x) {
    return x.type == LVAL_BIGNUM && mpz_cmp_ui(x.data.bignum, ULONG_MAX) > 0;
}
inline bool cnd_x_too_big_for_ul(struct lval x, struct lval y) {
    UNUSED(y);
    return _too_big_for_ul(x);
}
inline bool cnd_y_too_big_for_ul(struct lval x, struct lval y) {
    UNUSED(x);
    return _too_big_for_ul(y);
}

#endif
