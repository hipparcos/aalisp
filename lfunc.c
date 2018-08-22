#include "lfunc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "lerr.h"
#include "lenv.h"
#include "lbuiltin_condition.h"
#include "lbuiltin.h"

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
    lval_free(fun->args);
    fun->args = NULL;
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
    if (src->args) {
        dest->args = lval_alloc();
        lval_copy(dest->args, src->args);
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
    if (left->scope || right->scope) {
        CHECK(lenv_are_equal(left->scope, right->scope));
    }
    if (left->formals || right->formals) {
        CHECK(lval_are_equal(left->formals, right->formals));
    }
    if (left->body || right->body) {
        CHECK(lval_are_equal(left->body, right->body));
    }
    if (left->args || right->args) {
        CHECK(lval_are_equal(left->args, right->args));
    }
    return true;
}

static int lfunc_check_guards(
        const struct lfunc* fun,
        const struct lguard* guards, size_t guardc,
        const struct lval* args, struct lval* acc) {
    int s = 0;
    struct lerr* err = NULL;
    for (size_t g = 0; g < guardc; g++) {
        const struct lguard* guard = &guards[g];
        /* Guard applied on a specific argument. */
        if (guard->argn > 0) {
            struct lval* child = lval_alloc();
            lval_index(args, guard->argn-1, child);
            if (0 != (s = (guard->condition)(guard->param, fun, child, &err))) {
                lval_free(child);
                struct lerr* wrapped = lerr_propagate(err,
                        "argument %ld of `%s`", guard->argn, fun->symbol);
                lval_mut_err_ptr(acc, wrapped);
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
                if (0 != (s = (guard->condition)(guard->param, fun, child, &err))) {
                    s = a+1;
                    lval_free(child);
                    struct lerr* wrapped = lerr_propagate(err,
                            "argument %ld of `%s`", a+1, fun->symbol);
                    lval_mut_err_ptr(acc, wrapped);
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
        if (0 != (s = (guard->condition)(guard->param, fun, args, &err))) {
            struct lerr* wrapped = lerr_propagate(err, "`%s`", fun->symbol);
            lval_mut_err_ptr(acc, wrapped);
            break;
        }
    }
    return s;
}

static struct lenv* lfunc_prepare_env(
        const struct lfunc* fun, struct lenv* par, const struct lval* args) {
    if (fun->lisp_func) {
        struct lval* largs = lval_alloc();
        lval_copy(largs, args);
        struct lenv* env = fun->scope;
        lenv_set_parent(env, par);
        /* Define local variable. */
        size_t len = lval_len(fun->formals);
        for (size_t a = 0; a < len; a++) {
            struct lval* sym = lval_alloc();
            lval_index(fun->formals, a, sym);
            /* Special case when & is the argument right before last one. */
            if (a == len-2 && strcmp("&", lval_as_sym(sym)) == 0) {
                lval_index(fun->formals, a+1, sym);
                struct lval* list = lval_alloc();
                lfunc_exec(&lbuiltin_list, env, largs, list);
                lenv_put(env, sym, list);
                lval_free(list);
                lval_free(sym);
                break;
            }
            struct lval* arg = lval_pop(largs, 0);
            lenv_put(env, sym, arg);
            lval_free(sym);
            lval_free(arg);
        }
        lval_free(largs);
        return env;
    }
    return par;
}

#define min(a,b) ((a < b) ? a : b)
void lfunc_push_args(const struct lfunc* fun, const struct lval* args) {
    if (fun->args == 0) {
        return;
    }
    size_t len = lval_len(args);
    for (size_t a = 0; a < len; a++) {
        struct lval* arg = lval_alloc();
        lval_index(args, a, arg);
        lval_push(fun->args, arg);
        lval_free(arg);
    }
}

static struct lguard lbuitin_guards[] = {
    {.argn= -1, .condition= use_condition(must_have_max_argc)},
    {.argn= -1, .condition= use_condition(must_have_min_argc)},
    {.argn= -1, .condition= use_condition(must_have_func_ptr)},
};

int lfunc_exec(const struct lfunc* fun, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    if (!fun) {
        struct lerr* err = lerr_throw(LERR_EVAL, "nil can't be executed");
        lval_mut_err_ptr(acc, err);
        return -1;
    }
    /* Partial application. */
    if (fun->args) {
        lfunc_push_args(fun, args);
        int argc = lval_len(fun->args);
        if (argc < fun->min_argc) {
            lval_mut_func(acc, fun);
            return 0;
        }
        args = fun->args;
    }
    /* Guards */
    int s = 0;
    if (0 != (s = lfunc_check_guards(
                    fun, &lbuitin_guards[0], LENGTH(lbuitin_guards), args, acc))) {
        return s;
    }
    if (0 != (s = lfunc_check_guards(
                    fun, fun->guards, fun->guardc, args, acc))) {
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
