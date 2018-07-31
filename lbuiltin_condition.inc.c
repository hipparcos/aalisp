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
INLINE static int cnd_y_is_not_zero(const struct lval* x, const struct lval* y) {
    UNUSED(x);
    if (lval_is_zero(y)) return 2;
    return 0;
}

INLINE static bool _is_neg(const struct lval* x) {
    return lval_sign(x) < 0;
}
INLINE static int cnd_x_is_positive(const struct lval* x, const struct lval* y) {
    UNUSED(y);
    if (_is_neg(x)) return 1;
    return 0;
}

INLINE static int cnd_x_is_integral(const struct lval* x, const struct lval* y) {
    if (lval_type(x) == LVAL_NUM || lval_type(x) == LVAL_BIGNUM) return 0;
    return 1;
}
INLINE static int cnd_are_integral(const struct lval* x, const struct lval* y) {
    if (lval_type(x) != LVAL_NUM && lval_type(x) != LVAL_BIGNUM) return 1;
    if (lval_type(y) != LVAL_NUM && lval_type(y) != LVAL_BIGNUM) return 2;
    return 0;
}
INLINE static int cnd_are_numeric(const struct lval* x, const struct lval* y) {
    if (!(lval_is_numeric(x))) return 1;
    if (!(lval_is_numeric(y))) return 2;
    return 0;
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
static int cnd_x_is_ul(const struct lval* x, const struct lval* y) {
    UNUSED(y);
    if (_too_big_for_ul(x)) return 1;
    return 0;
}
static int cnd_y_is_ul(const struct lval* x, const struct lval* y) {
    UNUSED(x);
    if (_too_big_for_ul(y)) return 2;
    return 0;
}
