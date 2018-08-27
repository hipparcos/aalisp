#include "lbuiltin_func.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"
#include "lerr.h"
#include "lenv.h"
#include "leval.h"
#include "lfunc.h"
#include "lbuiltin.h"

#define UNUSED(x) (void)x

int lbi_func_if(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: boolean. */
    struct lval* boolean = lval_alloc();
    lval_index(args, 0, boolean);
    /* Retrieve arg 2 or 3: branch. */
    struct lval* branch = lval_alloc();
    if (lval_as_bool(boolean)) {
        lval_index(args, 1, branch);
    } else {
        lval_index(args, 2, branch);
    }
    /* Eval. */
    struct lval* wrap = lval_alloc();
    lval_mut_sexpr(wrap);
    lval_push(wrap, branch);
    int s = lfunc_exec(&lbuiltin_eval, env, wrap, acc);
    lval_free(wrap);
    /* Cleanup. */
    lval_free(boolean);
    lval_free(branch);
    return s;
}

int lbi_func_loop(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: boolean. */
    struct lval* branch_cond = lval_alloc();
    lval_index(args, 0, branch_cond);
    struct lval* wrap_cond = lval_alloc();
    lval_mut_sexpr(wrap_cond);
    lval_push(wrap_cond, branch_cond);
    /* Retrieve arg 2 or 3: branch. */
    struct lval* branch_body = lval_alloc();
    lval_index(args, 1, branch_body);
    struct lval* wrap_body = lval_alloc();
    lval_mut_sexpr(wrap_body);
    lval_push(wrap_body, branch_body);
    /* Loop. */
    int s = 0;
    struct lval* boolean = lval_alloc();
    while (0 == (s = lfunc_exec(&lbuiltin_eval, env, wrap_cond, boolean)) && lval_as_bool(boolean)) {
        s = lfunc_exec(&lbuiltin_eval, env, wrap_body, acc);
        if (s != 0) {
            break;
        }
    }
    lval_free(boolean);
    /* Cleanup. */
    lval_free(wrap_cond);
    lval_free(wrap_body);
    lval_free(branch_cond);
    lval_free(branch_body);
    return s;
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

int lbi_func_index(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: index. */
    struct lval* idx = lval_alloc();
    lval_index(args, 0, idx);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    /* Index. */
    long i = 0;
    lval_as_num(idx, &i);
    if (i < 0) {
        i *= -1;
        i = lval_len(list) - i;
    }
    lval_index(list, i, acc);
    /* Cleanup. */
    lval_free(idx);
    lval_free(list);
    return 0;
}

int lbi_func_elem(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: elem. */
    struct lval* elem = lval_alloc();
    lval_index(args, 0, elem);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    /* Elem. */
    size_t len = lval_len(list);
    struct lval* child = lval_alloc();
    lval_mut_bool(acc, false);
    for (size_t e = 0; e < len; e++) {
        lval_index(list, e, child);
        if (lval_are_equal(elem, child)) {
            lval_mut_bool(acc, true);
            break;
        }
    }
    lval_free(child);
    /* Cleanup. */
    lval_free(elem);
    lval_free(list);
    return 0;
}

int lbi_func_take(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: index. */
    struct lval* idx = lval_alloc();
    lval_index(args, 0, idx);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    /* Take. */
    long i = 0;
    lval_as_num(idx, &i);
    size_t first = 0;
    size_t last = 0;
    size_t len = lval_len(list);
    if (i < 0) {
        i *= -1;
        first = ((size_t)i > len) ? 0 : len - i;
        last = len;
    } else {
        first = 0;
        last = i;
    }
    lval_copy_range(acc, list, first, last);
    /* Cleanup. */
    lval_free(idx);
    lval_free(list);
    return 0;
}

int lbi_func_drop(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: index. */
    struct lval* idx = lval_alloc();
    lval_index(args, 0, idx);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    /* Drop. */
    long i = 0;
    lval_as_num(idx, &i);
    size_t first = 0;
    size_t last = 0;
    size_t len = lval_len(list);
    if (i < 0) {
        i *= -1;
        first = 0;
        last = ((size_t)i > len) ? 0 : len - i;
    } else {
        first = i;
        last = len;
    }
    lval_copy_range(acc, list, first, last);
    /* Cleanup. */
    lval_free(idx);
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
    /* Init with correct type (for strings). */
    if (lval_len(acc) == 0) {
        switch (lval_type(args)) {
        case LVAL_STR:   lval_mut_str(acc, ""); break;
        case LVAL_SEXPR: lval_mut_sexpr(acc);   break;
        default:         lval_mut_qexpr(acc);   break;
        }
    }
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

int lbi_func_map(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: function. */
    struct lval* func = lval_alloc();
    lval_index(args, 0, func);
    const struct lfunc* func_ptr = lval_as_func(func);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    switch (lval_type(list)) {
        case LVAL_STR:   lval_mut_str(acc, ""); break;
        case LVAL_SEXPR: lval_mut_sexpr(acc);   break;
        default:         lval_mut_qexpr(acc);   break;
    }
    /* Map. */
    int s = 0;
    size_t len = lval_len(list);
    size_t len_bound = lval_len(func_ptr->args);
    struct lval* elem = lval_alloc();
    struct lval* res = lval_alloc();
    struct lval* wrap = lval_alloc();
    for (size_t e = 0; e < len; e++) {
        lval_index(list, e, elem);
        lval_clear(res);
        lval_clear(wrap);
        lval_mut_qexpr(wrap);
        lval_push(wrap, elem);
        /* Elem is added to func_ptr->args. */
        s = lfunc_exec(func_ptr, env, wrap, res);
        if (s != 0) {
            lval_dup(acc, res);
            s = e+1;
            break;
        }
        /* Drop last argument. */
        lval_drop(func_ptr->args, len_bound);
        lval_push(acc, res);
    }
    lval_free(elem);
    lval_free(res);
    lval_free(wrap);
    /* Cleanup. */
    lval_free(func);
    lval_free(list);
    return s;
}

int lbi_func_filter(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: function. */
    struct lval* func = lval_alloc();
    lval_index(args, 0, func);
    const struct lfunc* func_ptr = lval_as_func(func);
    /* Retrieve arg 2: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 1, list);
    switch (lval_type(list)) {
        case LVAL_STR:   lval_mut_str(acc, ""); break;
        case LVAL_SEXPR: lval_mut_sexpr(acc);   break;
        default:         lval_mut_qexpr(acc);   break;
    }
    /* Filter. */
    int s = 0;
    size_t len = lval_len(list);
    size_t len_bound = lval_len(func_ptr->args);
    struct lval* elem = lval_alloc();
    struct lval* res = lval_alloc();
    struct lval* wrap = lval_alloc();
    for (size_t e = 0; e < len; e++) {
        lval_index(list, e, elem);
        lval_clear(res);
        lval_clear(wrap);
        lval_mut_qexpr(wrap);
        lval_push(wrap, elem);
        /* Elem is added to func_ptr->args. */
        s = lfunc_exec(func_ptr, env, wrap, res);
        if (s != 0) {
            lval_dup(acc, res);
            s = e+1;
            break;
        }
        /* Drop last argument. */
        lval_drop(func_ptr->args, len_bound);
        if (lval_as_bool(res)) {
            lval_push(acc, elem);
        }
    }
    lval_free(elem);
    lval_free(res);
    lval_free(wrap);
    /* Cleanup. */
    lval_free(func);
    lval_free(list);
    return s;
}

int lbi_func_fold(struct lenv* env, const struct lval* args, struct lval* acc) {
    /* Retrieve arg 1: function. */
    struct lval* func = lval_alloc();
    lval_index(args, 0, func);
    const struct lfunc* func_ptr = lval_as_func(func);
    /* Retrieve arg 2: initial value. */
    struct lval* init = lval_alloc();
    lval_index(args, 1, init);
    /* Retrieve arg 3: list. */
    struct lval* list = lval_alloc();
    lval_index(args, 2, list);
    /* Fold. */
    int s = 0;
    size_t len = lval_len(list);
    size_t len_bound = lval_len(func_ptr->args);
    struct lval* elem = lval_alloc();
    struct lval* wrap = lval_alloc();
    lval_dup(acc, init);
    for (size_t e = 0; e < len; e++) {
        lval_index(list, e, elem);
        lval_clear(wrap);
        lval_mut_qexpr(wrap);
        lval_push(wrap, acc);
        lval_push(wrap, elem);
        /* Elem is added to func_ptr->args. */
        s = lfunc_exec(func_ptr, env, wrap, acc);
        if (s != 0) {
            s = e+1;
            break;
        }
        /* Drop last argument. */
        lval_drop(func_ptr->args, len_bound); // arg.
        lval_drop(func_ptr->args, len_bound); // acc.
    }
    lval_free(elem);
    lval_free(wrap);
    /* Cleanup. */
    lval_free(func);
    lval_free(init);
    lval_free(list);
    return s;
}

static int lbi_def(struct lenv* env,
        bool (*def)(struct lenv*, const struct lval*, const struct lval*),
        const struct lval* symbols, const struct lval* values,
        struct lval* result) {
    /* Define symbols. */
    size_t len = lval_len(symbols);
    for (size_t c = 0; c < len; c++) {
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
    return (leval(env, args, acc)) ? 0 : 1;
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
    if (argc >= 2) {
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
    func.symbol = "λ";
    func.min_argc = min_argc;
    func.max_argc = max_argc;
    func.lisp_func = true;
    func.func = lbi_func_call;
    lfunc_init(&func);
    lval_dup(func.formals, formals);
    lval_dup(func.body, body);
    /* Put lambda in acc. */
    lval_mut_func(acc, &func);
    /* Cleanup. */
    lfunc_clear(&func);
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
    /* Set symbol. */
    struct lfunc* fun_ptr = lval_as_func(acc);
    lfunc_set_symbol(fun_ptr, lval_as_sym(name));
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

int lbi_func_partial(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    struct lval* largs = lval_alloc();
    lval_copy(largs, args);
    lval_mut_qexpr(largs);
    /* Retrieve arg 1: function pointer. */
    struct lval* func = lval_pop(largs, 0);
    lval_dup(acc, func);
    struct lfunc* fun_ptr = lval_as_func(acc);
    lfunc_push_args(fun_ptr, largs);
    /* Cleanup. */
    lval_free(func);
    lval_free(largs);
    return 0;
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

int lbi_func_load(struct lenv* env, const struct lval* args, struct lval* acc) {
    struct lval* filev = lval_alloc();
    /* Evaluates each file given as argument. */
    size_t len = lval_len(args);
    for (size_t a = 0; a < len; a++) {
        lval_index(args, a, filev);
        const char* filename = lval_as_str(filev);
        FILE* f = fopen(filename, "r");
        if (!f) {
            struct lerr* err = lerr_throw(LERR_ENOENT,
                    "file `%s` not found", filename);
            lval_mut_err(acc, err);
            lval_free(filev);
            return a+1;
        }
        leval_from_file(env, f, acc);
        fclose(f);
    }
    lval_free(filev);
    return 0;
}

int lbi_func_error(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: error string. */
    struct lval* errv = lval_alloc();
    lval_index(args, 0, errv);
    /* Error. */
    struct lerr* err = lerr_throw(LERR_LISP_ERROR, "%s", lval_as_str(errv));
    lval_mut_err_ptr(acc, err);
    /* Cleanup. */
    lval_free(errv);
    return 1;
}

int lbi_func_debug_env(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(args);
    return lenv_as_list(env, acc) == 0;
}

int lbi_func_debug_fun(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: function. */
    struct lval* func = lval_alloc();
    lval_index(args, 0, func);
    const struct lfunc* func_ptr = lval_as_func(func);
    /* Debug. */
    lval_mut_qexpr(acc);
    lval_push(acc, func_ptr->formals);
    lval_push(acc, func_ptr->args);
    if (func_ptr->lisp_func) {
        lval_push(acc, func_ptr->body);
    } else {
        struct lval* builtin = lval_alloc();
        lval_mut_func(builtin, func_ptr);
        struct lfunc* builtin_ptr = lval_as_func(builtin);
        lval_clear(builtin_ptr->args);
        lval_mut_qexpr(builtin_ptr->args);
        lval_push(acc, builtin);
        lval_free(builtin);
    }
    /* Cleanup. */
    lval_free(func);
    return 0;
}

int lbi_func_debug_val(struct lenv* env, const struct lval* args, struct lval* acc) {
    UNUSED(env);
    /* Retrieve arg 1: list of lval. */
    struct lval* list = lval_alloc();
    lval_index(args, 0, list);
    /* Debug. */
    lval_mut_qexpr(acc);
    size_t len = lval_len(list);
    struct lval* sym = lval_alloc();
    struct lval* result = lval_alloc();
    struct lval* type_sym = lval_alloc();
    struct lval* type_result = lval_alloc();
    struct lval* wrap = lval_alloc();
    for (size_t s = 0; s < len; s++) {
        lval_index(list, s, sym);
        lval_clear(result);
        lval_clear(wrap);
        lval_mut_qexpr(wrap);
        /* Eval. */
        leval(env, sym, result);
        /* Type as string. */
        lval_mut_str(type_sym, lval_type_string(lval_type(sym)));
        lval_mut_str(type_result, lval_type_string(lval_type(result)));
        /* Result. */
        struct lval* push_into = acc;
        if (len > 1) {
            push_into = wrap;
        }
        lval_push(push_into, type_sym);
        lval_push(push_into, sym);
        lval_push(push_into, type_result);
        lval_push(push_into, result);
        if (len > 1) {
            lval_push(acc, result);
        }
    }
    /* Cleanup. */
    lval_free(sym);
    lval_free(result);
    lval_free(type_sym);
    lval_free(type_result);
    lval_free(wrap);
    lval_free(list);
    return 0;
}
