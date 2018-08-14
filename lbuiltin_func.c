#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdio.h>

#include "lsym.h"
#include "lval.h"
#include "leval.h"

int lbi_cond_qexpr(const struct lval* x, const struct lval* y) {
    (void)y;
    return (lval_type(x) == LVAL_QEXPR) ? 0 : 1;
}

int lbi_cond_qexpr_or_nil(const struct lval* x, const struct lval* y) {
    (void)y;
    return (lval_type(x) == LVAL_QEXPR || lval_is_nil(x)) ? 0 : 1;
}

int lbi_cond_list(const struct lval* x, const struct lval* y) {
    (void)y;
    return (lval_len(x) > 0) ? 0 : 1;
}

int lbi_func_head(struct lval* acc, const struct lval* x) {
    lval_index(x, 0, acc);
    return 0;
}

int lbi_func_tail(struct lval* acc, const struct lval* x) {
    lval_copy(acc, x);
    struct lval* child = lval_pop(acc, 0);
    lval_free(child);
    return 0;
}

int lbi_func_join(struct lval* acc, const struct lval* x) {
    size_t len = lval_len(x);
    struct lval* child = lval_alloc();
    for (size_t c = 0; c < len; c++) {
        lval_index(x, c, child);
        lval_push(acc, child);
    }
    lval_free(child);
    return 0;
}

int lbi_func_list(struct lval* acc, const struct lval* x) {
    if (lval_type(acc) != LVAL_QEXPR) {
        lval_mut_qexpr(acc);
    }
    struct lval* tmp = lval_alloc();
    lval_dup(tmp, x);
    lval_push(acc, x);
    /* lval_free(tmp); */ // Must comment because of refc bug. TODO fix refc bug.
    return 0;
}

int lbi_func_eval(struct lval* acc, const struct lval* x) {
    struct lval* r = lval_alloc();
    bool s = leval(x, r);
    lval_copy(acc, r); // Must copy because of refc bug. TODO fix refc bug.
    lval_free(r);
    return !s;
}
