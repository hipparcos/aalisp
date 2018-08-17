#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdio.h>

#include "lval.h"
#include "lenv.h"
#include "leval.h"

#define UNUSED(x) (void)x

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

int lbi_func_head(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    /* Head. */
    lval_index(arg, 0, acc);
    lval_free(arg);
    return 0;
}

int lbi_func_tail(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    lval_copy(acc, arg);
    lval_free(arg);
    /* Tail. */
    struct lval* child = lval_pop(acc, 0);
    lval_free(child);
    return 0;
}

int lbi_func_init(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    lval_copy(acc, arg);
    lval_free(arg);
    /* Init. */
    size_t len = lval_len(acc);
    struct lval* child = lval_pop(acc, len-1);
    lval_free(child);
    return 0;
}

int lbi_func_cons(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    /* Cons. */
    lval_dup(acc, list);
    lval_cons(acc, arg);
    lval_free(arg);
    lval_free(list);
    return 0;
}

int lbi_func_len(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    /* Len. */
    size_t len = lval_len(arg);
    lval_free(arg);
    lval_mut_num(acc, len);
    return 0;
}

int lbi_func_join(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    size_t len = lval_len(args);
    struct lval* child = lval_alloc();
    for (size_t c = 0; c < len; c++) {
        lval_index(args, c, child);
        lval_push(acc, child);
    }
    lval_free(child);
    return 0;
}

int lbi_func_list(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    struct lval* tmp = lval_alloc();
    lval_dup(tmp, args);
    lval_push(acc, args);
    lval_free(tmp);
    return 0;
}

int lbi_func_eval(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* arg = lval_alloc();
    lval_index(args, 0, arg);
    /* Eval. */
    struct lval* r = lval_alloc();
    bool s = leval(env, arg, r);
    lval_free(arg);
    lval_copy(acc, r);
    lval_free(r);
    return !s;
}

int lbi_func_def(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1. */
    struct lval* symbols = lval_alloc();
    lval_index(args, 0, symbols);
    /* Check len. */
    size_t lensyms = lval_len(symbols);
    size_t lenargs = lval_len(args)-1;
    if (lenargs != lensyms) {
        if (lenargs < lensyms) {
            lval_mut_err(acc, LERR_TOO_FEW_ARGS);
        } else {
            lval_mut_err(acc, LERR_TOO_MANY_ARGS);
        }
        lval_free(symbols);
        return -1;
    }
    /* Define symbols. */
    for (size_t c = 1; c < lenargs+1; c++) {
        struct lval* sym = lval_alloc();
        struct lval* value = lval_alloc();
        lval_index(symbols, c-1, sym);
        lval_index(args, c, value);
        if (!lenv_put(env, sym, value)) {
            lval_free(value);
            lval_free(sym);
            return -1;
        }
        lval_free(value);
        lval_free(sym);
    }
    lval_dup(acc, symbols);
    /* Cleanup. */
    lval_free(symbols);
    return 0;
}
