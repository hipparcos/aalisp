#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"
#include "lenv.h"
#include "leval.h"
#include "lfunc.h"
#include "lbuiltin.h"

#define UNUSED(x) (void)x

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
    lval_drop(acc, 0);
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
    lval_drop(acc, len-1);
    return 0;
}

int lbi_func_last(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1. */
    struct lval* list = lval_alloc();
    lval_index(args, 0, list);
    /* Last. */
    size_t len = lval_len(list);
    lval_index(list, len-1, acc);
    lval_free(list);
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
    /* Mut Q-Expression in S-Expr. */
    if (lval_type(arg) == LVAL_QEXPR) {
        struct lval* cpy = lval_alloc();
        lval_copy(cpy, arg);
        lval_mut_sexpr(cpy);
        lval_free(arg);
        arg = cpy;
    }
    /* Eval. */
    struct lval* r = lval_alloc();
    bool s = leval(env, arg, r);
    lval_free(arg);
    lval_copy(acc, r);
    lval_free(r);
    return !s;
}

static int lbi_def(struct lenv* env,
        bool (*def)(struct lenv*, const struct lval*, const struct lval*),
        const struct lval* symbols, const struct lval* values,
        struct lval* result) {
    /* Check len. */
    size_t lensyms = lval_len(symbols);
    size_t lenvals = lval_len(values);
    if (lenvals != lensyms) {
        enum lerr_code code = 0;
        if (lenvals < lensyms) {
            code = LERR_TOO_FEW_ARGS;
        } else {
            code = LERR_TOO_MANY_ARGS;
        }
        struct lerr* err = lerr_throw(code,
                "def called with %ld symbol(s) and %ld value(s)", lensyms, lenvals);
        lval_mut_err_ptr(result, err);
        return -1;
    }
    /* Define symbols. */
    for (size_t c = 0; c < lensyms; c++) {
        struct lval* sym = lval_alloc();
        struct lval* value = lval_alloc();
        lval_index(symbols, c, sym);
        lval_index(values, c, value);
        if (!def(env, sym, value)) {
            lval_free(value);
            lval_free(sym);
            return -1;
        }
        lval_free(value);
        lval_free(sym);
    }
    lval_dup(result, symbols);
    return 0;
}

int lbi_func_def(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: list of symbols. */
    struct lval* symbols = lval_alloc();
    lval_index(args, 0, symbols);
    /* Retrieve arg 2...n: list of values. */
    struct lval* values = lval_alloc();
    lval_copy(values, args);
    lval_drop(values, 0); // Drops list of symbols.
    /* Def. */
    int s = lbi_def(env, lenv_def, symbols, values, acc);
    /* Cleanup. */
    lval_free(symbols);
    lval_free(values);
    return s;
}

int lbi_func_override(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: list of symbols. */
    struct lval* symbols = lval_alloc();
    lval_index(args, 0, symbols);
    /* Retrieve arg 2...n: list of values. */
    struct lval* values = lval_alloc();
    lval_copy(values, args);
    lval_drop(values, 0); // Drops list of symbols.
    /* Def. */
    int s = lbi_def(env, lenv_override, symbols, values, acc);
    /* Cleanup. */
    lval_free(symbols);
    lval_free(values);
    return s;
}

int lbi_func_put(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: list of symbols. */
    struct lval* symbols = lval_alloc();
    lval_index(args, 0, symbols);
    /* Retrieve arg 2...n: list of values. */
    struct lval* values = lval_alloc();
    lval_copy(values, args);
    lval_drop(values, 0); // Drops list of symbols.
    /* Def. */
    int s = lbi_def(env, lenv_put, symbols, values, acc);
    /* Cleanup. */
    lval_free(symbols);
    lval_free(values);
    return s;
}

static int lbi_func_call(struct lenv* env, const struct lval* args, struct lval* acc) {
    return leval(env, args, acc);
}

int lbi_func_lambda(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: list of formals. */
    struct lval* formals = lval_alloc();
    lval_index(args, 0, formals);
    /* Retrieve arg 1: list of sexpr. */
    struct lval* body = lval_alloc();
    lval_index(args, 1, body);
    /* Argument count. */
    size_t argc = lval_len(formals);
    int min_argc = argc;
    int max_argc = argc;
    /* Special case when & is the argument right before last one. */
    if (argc >= 3) {
        struct lval* bef_last = lval_alloc();
        lval_index(formals, argc - 2, bef_last);
        if (strcmp("&", lval_as_sym(bef_last)) == 0) {
            min_argc -= 2; // Remove &; last is optional.
            max_argc = -1;
        }
        lval_free(bef_last);
    }
    /* Create lfunc. */
    struct lfunc func = {0};
    func.symbol = "lisp defined function";
    func.min_argc = min_argc;
    func.max_argc = max_argc;
    func.lisp_func = true;
    func.func = lbi_func_call;
    func.scope = lenv_alloc();
    func.formals = lval_alloc();
    lval_dup(func.formals, formals);
    func.body = lval_alloc();
    lval_copy(func.body, body);
    lval_mut_sexpr(func.body); // Works because it's a brand new copy.
    func.args = lval_alloc();
    lval_mut_sexpr(func.args);
    /* Put lambda in acc. */
    lval_mut_func(acc, &func);
    /* Cleanup. */
    lval_free(formals);
    lval_free(body);
    return 0;
}

int lbi_func_fun(struct lenv* env, const struct lval* args, struct lval* acc) {
    int s = 0;
    /* Retrieve arg 1: list of symbols. */
    struct lval* symbols = lval_alloc();
    lval_index(args, 0, symbols);
    /* Retrieve arg 1 first symbol: function name. */
    struct lval* name = lval_pop(symbols, 0);
    /* Retrieve arg 1: formals. */
    struct lval* formals = symbols; // For clarity.
    /* Retrieve arg 2: body. */
    struct lval* body = lval_alloc();
    lval_index(args, 1, body);
    /* Lambda definition. */
    struct lval* lambda = lval_alloc();
    lval_mut_qexpr(lambda);
    lval_push(lambda, formals);
    lval_push(lambda, body);
    s = lfunc_exec(&lbuiltin_lambda, env, lambda, acc);
    lval_free(lambda);
    if (s != 0) {
        lval_free(name);
        lval_free(formals);
        lval_free(body);
        return s + 1; // step over first arg.
    }
    /* Environment registration. */
    struct lval* def = lval_alloc();
    lval_mut_qexpr(def);
    struct lval* def_list = lval_alloc();
    lval_mut_qexpr(def_list);
    lval_push(def_list, name);
    lval_push(def, def_list);
    lval_push(def, acc);
    s = lfunc_exec(&lbuiltin_def, env, def, acc);
    lval_free(def_list);
    lval_free(def);
    /* Cleanup. */
    lval_free(name);
    lval_free(formals);
    lval_free(body);
    return s;
}

int lbi_func_pack(struct lenv* env, const struct lval* args, struct lval* acc) {
    struct lval* largs = lval_alloc();
    lval_copy(largs, args);
    lval_mut_qexpr(largs);
    /* Retrieve arg 1: function pointer. */
    struct lval* func_ptr = lval_pop(largs, 0);
    /* Remaining args: function arguments. */
    struct lval* func_args = lval_alloc(); // For clarity.
    lval_mut_qexpr(func_args);
    lval_push(func_args, largs);
    /* Create Q-Expression. */
    int s = lfunc_exec(lval_as_func(func_ptr), env, func_args, acc);
    /* Cleanup. */
    lval_free(func_ptr);
    lval_free(func_args);
    lval_free(largs);
    return s;
}

int lbi_func_unpack(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: function pointer. */
    struct lval* func_ptr = lval_alloc();
    lval_index(args, 0, func_ptr);
    /* Retrieve arg 2: function arguments. */
    struct lval* func_args = lval_alloc();
    lval_index(args, 1, func_args);
    /* Eval. */
    int s = lfunc_exec(lval_as_func(func_ptr), env, func_args, acc);
    /* Cleanup. */
    lval_free(func_ptr);
    lval_free(func_args);
    return s;
}

int lbi_func_print(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    size_t len = lval_len(args);
    struct lval* arg = lval_alloc();
    for (size_t a = 0; a < len; a++) {
        if (a) {
            fputc(' ', stdout);
        }
        lval_index(args, a, arg);
        lval_print_to(arg, stdout);
    }
    fputc('\n', stdout);
    lval_free(arg);
    lval_mut_nil(acc);
    return 0;
}
