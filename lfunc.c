#include "lfunc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "lenv.h"
#include "lbuiltin_condition.h"

#define LENGTH(arr) sizeof(arr)/sizeof(arr[0])

struct lfunc* lfunc_alloc(void) {
    struct lfunc* fun = calloc(1, sizeof(struct lfunc));
    return fun;
}

static void lfunc_clear(struct lfunc* fun) {
    lenv_free(fun->scope);
    fun->scope = NULL;
    lval_free(fun->formals);
    fun->formals = NULL;
    lval_free(fun->body);
    fun->body = NULL;
}

void lfunc_free(struct lfunc* fun) {
    if (!fun) {
        return;
    }
    lfunc_clear(fun);
    free(fun);
}

bool lfunc_copy(struct lfunc* dest, const struct lfunc* src) {
    if (!dest || !src) {
        return false;
    }
    lfunc_clear(dest);
    memcpy(dest, src, sizeof(struct lfunc));
    if (src->scope) {
        dest->scope = lenv_alloc();
        lenv_copy(dest->scope, src->scope);
    }
    if (src->formals) {
        dest->formals = lval_alloc();
        lval_copy(dest->formals, src->formals);
    }
    if (src->body) {
        dest->body = lval_alloc();
        lval_copy(dest->body, src->body);
    }
    return true;
}

#define CHECK(cond) if (!(cond)) return false;
bool lfunc_are_equal(const struct lfunc* left, const struct lfunc* right) {
    if (left == right) {
        return true;
    }
    CHECK(left && right);
    CHECK(strcmp(left->symbol, right->symbol) == 0);
    CHECK(left->min_argc == right->min_argc);
    CHECK(left->max_argc == right->max_argc);
    CHECK(left->accumulator == right->accumulator);
    CHECK(left->guards == right->guards);
    CHECK(left->guardc == right->guardc);
    CHECK(left->init_neutral == right->init_neutral);
    CHECK(lval_are_equal(left->neutral, right->neutral));
    CHECK(left->func == right->func);
    CHECK(lenv_are_equal(left->scope, right->scope));
    CHECK(lval_are_equal(left->formals, right->formals));
    CHECK(lval_are_equal(left->body, right->body));
    return true;
}

static int lfunc_check_guards(
        const struct lfunc* fun,
        const struct lguard* guards, size_t guardc,
        const struct lenv* env, const struct lval* args, struct lval* acc) {
    int s = 0;
    const struct lguard* guard = NULL;
    for (size_t g = 0; g < guardc; g++) {
        guard = &guards[g];
        /* Guard applied on a specific argument. */
        if (guard->argn > 0) {
            struct lval* child = lval_alloc();
            lval_index(args, guard->argn-1, child);
            if (0 != (s = (guard->condition)(fun, env, child))) {
                lval_free(child);
                break;
            }
            lval_free(child);
            continue;
        }
        /* Guard applied on each args. */
        if (guard->argn == 0) {
            size_t len = lval_len(args);
            for (size_t a = 0; a < len; a++) {
                struct lval* child = lval_alloc();
                lval_index(args, a, child);
                if (0 != (s = (guard->condition)(fun, env, child))) {
                    lval_free(child);
                    break;
                }
                lval_free(child);
            }
            if (s != 0) {
                break;
            }
            continue;
        }
        /* Guard applied on all args. */
        // guard->argn == -1;
        if (0 != (s = (guard->condition)(fun, env, args))) {
            break;
        }
    }
    if (s != 0 && guard) {
        lval_mut_err(acc, guard->error);
        return s;
    }
    return 0;
}

static struct lenv* lfunc_prepare_env(
        const struct lfunc* fun, struct lenv* par, const struct lval* args) {
    if (fun->lisp_func) {
        struct lenv* env = fun->scope;
        lenv_set_parent(env, par);
        /* Define local variable. */
        size_t len = lval_len(fun->formals);
        for (size_t a = 0; a < len; a++) {
            struct lval* sym = lval_alloc();
            struct lval* arg = lval_alloc();
            lval_index(fun->formals, a, sym);
            lval_index(args, a, arg);
            lenv_put(env, sym, arg);
            lval_free(sym);
            lval_free(arg);
        }
        return env;
    }
    return par;
}

static struct lguard lbuitin_guards[] = {
    {.condition= lbi_cond_max_argc,     .argn= -1, .error= LERR_TOO_MANY_ARGS},
    {.condition= lbi_cond_min_argc,     .argn= -1, .error= LERR_TOO_FEW_ARGS},
    {.condition= lbi_cond_func_pointer, .argn= -1, .error= LERR_EVAL},
};

int lfunc_exec(const struct lfunc* fun, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    if (!fun) {
        lval_mut_err(acc, LERR_EVAL);
        return -1;
    }
    /* Guards */
    int s = 0;
    if (0 != (s = lfunc_check_guards(fun,
                    &lbuitin_guards[0], LENGTH(lbuitin_guards),
                    env, args, acc))) {
        return s;
    }
    if (0 != (s = lfunc_check_guards(fun,
                    fun->guards, fun->guardc,
                    env, args, acc))) {
        return s;
    }
    /* Prepare environnement. */
    env = lfunc_prepare_env(fun, env, args);
    if (fun->lisp_func) {
        args = fun->body;
    }
    /* Builtin argument execution. */
    if (!fun->accumulator) {
        return fun->func(env, args, acc);
    }
    /* Builtin accumulator execution. */
    size_t len = lval_len(args);
    if (len == 1) {
        /* Special case for unary operations. */
        struct lval* lx = lval_alloc();
        lval_index(args, 0, lx);      // lx is the first argument.
        lval_copy(acc, fun->neutral); // acc is the neutral element.
        int s = fun->func(env, lx, acc);
        lval_free(lx);
        return s;
    }
    if (fun->init_neutral) {
        /* Init acc with neutral. */
        lval_copy(acc, fun->neutral);
    } else {
        /* Init acc with first argument. */
        lval_index(args, 0, acc);
    }
    for (size_t c = (fun->init_neutral) ? 0 : 1; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(args, c, child);
        int err = fun->func(env, child, acc);
        /* Break on error. */
        if (err != 0) {
            if (err == -1) {
                s = err;
            } else {
                s = c + 1;
            }
            break;
        }
        lval_free(child);
    }
    return s;
}
