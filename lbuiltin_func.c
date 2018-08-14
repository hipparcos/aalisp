#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdio.h>

#include "lsym.h"
#include "lval.h"

int lbi_cond_qexpr(const struct lval* x, const struct lval* y) {
    (void)y;
    return (lval_type(x) == LVAL_QEXPR) ? 0 : 1;
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
    lval_pop(acc, 0);
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
