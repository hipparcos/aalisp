#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdio.h>

#include "lsym.h"
#include "lval.h"
#include "leval.h"

int lbi_cond_qexpr(const struct lval* x) {
    return (lval_type(x) == LVAL_QEXPR) ? 0 : 1;
}

int lbi_cond_qexpr_or_nil(const struct lval* x) {
    return (lval_type(x) == LVAL_QEXPR || lval_is_nil(x)) ? 0 : 1;
}

int lbi_cond_list(const struct lval* x) {
    return (lval_len(x) > 0) ? 0 : 1;
}

int lbi_cond_qexpr_all(const struct lval* x) {
    size_t len = lval_len(x);
    for (size_t c = 0; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(x, c, child);
        if (lval_type(child) != LVAL_QEXPR) {
            lval_free(child);
            return c + 1;
        }
        lval_free(child);
    }
    return 0;
}

int lbi_cond_list_all(const struct lval* x) {
    size_t len = lval_len(x);
    for (size_t c = 0; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(x, c, child);
        if (!lval_is_list(child)) {
            lval_free(child);
            return c + 1;
        }
        lval_free(child);
    }
    return 0;
}

int lbi_func_head(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    /* Head. */
    lval_index(arg, 0, acc);
    lval_free(arg);
    return 0;
}

int lbi_func_tail(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    lval_copy(acc, arg);
    lval_free(arg);
    /* Tail. */
    struct lval* child = lval_pop(acc, 0);
    lval_free(child);
    return 0;
}

int lbi_func_init(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    lval_copy(acc, arg);
    lval_free(arg);
    /* Init. */
    size_t len = lval_len(acc);
    struct lval* child = lval_pop(acc, len-1);
    lval_free(child);
    return 0;
}

int lbi_func_cons(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    struct lval* list = lval_alloc();
    lval_index(x, 1, list);
    /* Cons. */
    lval_dup(acc, list);
    lval_cons(acc, arg);
    lval_free(arg);
    lval_free(list);
    return 0;
}

int lbi_func_len(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    /* Len. */
    size_t len = lval_len(arg);
    lval_free(arg);
    lval_mut_num(acc, len);
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
    struct lval* tmp = lval_alloc();
    lval_dup(tmp, x);
    lval_push(acc, x);
    lval_free(tmp); // Must comment because of refc bug. TODO fix refc bug.
    return 0;
}

int lbi_func_eval(struct lval* acc, const struct lval* x) {
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(x, 0, arg);
    /* Eval. */
    struct lval* r = lval_alloc();
    bool s = leval(arg, r);
    lval_free(arg);
    lval_copy(acc, r); // Must copy because of refc bug. TODO fix refc bug.
    lval_free(r);
    return !s;
}
