#include <limits.h>

#include "vendor/mini-gmp/mini-gmp.h"
#include "lval.h"

/* Macros */
#define UNUSED(x) (void)(x)

#ifdef OPTIM
#define INLINE inline
#else
#define INLINE
#endif

/* Condition functions. */
INLINE static bool cnd_y_is_zero(const struct lval* x, const struct lval* y) {
    UNUSED(x);
    return lval_is_zero(y);
}

INLINE static bool cnd_either_is_dbl(const struct lval* x, const struct lval* y) {
    return lval_type(x) == LVAL_DBL || lval_type(y) == LVAL_DBL;
}

INLINE static bool _is_neg(const struct lval* x) {
    return lval_sign(x) < 0;
}
INLINE static bool cnd_x_is_neg(const struct lval* x, const struct lval* y) {
    UNUSED(y);
    return _is_neg(x);
}

INLINE static bool cnd_x_is_not_numeric(const struct lval* x, const struct lval* y) {
    return !lval_is_numeric(x);
}
INLINE static bool cnd_are_not_numeric(const struct lval* x, const struct lval* y) {
    return !(lval_is_numeric(x) && lval_is_numeric(y));
}

static bool _too_big_for_ul(const struct lval* x) {
    if (lval_type(x) != LVAL_BIGNUM) {
        return false;
    }
    mpz_t r;
    mpz_init(r);
    lval_as_bignum(x, r);
    bool result = mpz_cmp_ui(r, ULONG_MAX) > 0;
    mpz_clear(r);
    return result;
}
static bool cnd_x_too_big_for_ul(const struct lval* x, const struct lval* y) {
    UNUSED(y);
    return _too_big_for_ul(x);
}
static bool cnd_y_too_big_for_ul(const struct lval* x, const struct lval* y) {
    UNUSED(x);
    return _too_big_for_ul(y);
}
