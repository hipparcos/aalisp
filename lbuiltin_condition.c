#include "lbuiltin_condition.h"

#include <limits.h>

#define UNUSED(x) (void)x

int lbi_cond_max_argc(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args) {
    UNUSED(sym); UNUSED(env);
    size_t len = lval_len(args);
    int max = sym->max_argc;
    if (max != -1 && (int)len > max) {
        return -1;
    }
    return 0;
}

int lbi_cond_min_argc(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args) {
    UNUSED(sym); UNUSED(env);
    size_t len = lval_len(args);
    int min = sym->min_argc;
    if (min != -1 && (int)len < min) {
        return -1;
    }
    return 0;
}

int lbi_cond_func_pointer(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args) {
    UNUSED(env); UNUSED(args);
    if (!sym->accumulator && !sym->func) {
        return -1;
    }
    return 0;
}
int lbi_cond_is_not_zero(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (!lval_is_zero(arg)) ? 0 : 1;
}

int lbi_cond_is_positive(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (lval_sign(arg) > 0) ? 0 : 1;
}

int lbi_cond_is_integral(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return ((lval_type(arg) == LVAL_NUM || lval_type(arg) == LVAL_BIGNUM)) ? 0 : 1;
}

int lbi_cond_is_numeric(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (lval_is_numeric(arg)) ? 0 : 1;
}

static bool _too_big_for_ul(const struct lval* arg) {
    if (lval_type(arg) != LVAL_BIGNUM) {
        return false;
    }
    mpz_t r;
    mpz_init(r);
    lval_as_bignum(arg, r);
    bool result = mpz_cmp_ui(r, ULONG_MAX) > 0;
    mpz_clear(r);
    return result;
}
int lbi_cond_x_is_ul(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (!_too_big_for_ul(arg)) ? 0 : 1;
}

int lbi_cond_qexpr(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (lval_type(arg) == LVAL_QEXPR) ? 0 : 1;
}

int lbi_cond_qexpr_or_nil(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (lval_type(arg) == LVAL_QEXPR || lval_is_nil(arg)) ? 0 : 1;
}

int lbi_cond_list(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    return (lval_len(arg) > 0) ? 0 : 1;
}

int lbi_cond_list_of_sym(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg) {
    UNUSED(sym); UNUSED(env);
    if (!lval_is_list(arg)) {
        return 1;
    }
    size_t len = lval_len(arg);
    for (size_t c = 0; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(arg, c, child);
        if (lval_type(child) != LVAL_SYM) {
            return 1;
        }
        lval_free(child);
    }
    return 0;
}
