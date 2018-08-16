#include "lenv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "lbuiltin.h"

/** lenv associates a symbol descriptor with a string. */
struct lenv {
    char**  syms;
    struct lval** vals;
    size_t len;
};

struct lenv* lenv_alloc(void) {
    struct lenv* env = calloc(1, sizeof(struct lenv));
    env->syms = NULL;
    env->vals = NULL;
    env->len = 0;
    return env;
}

void lenv_free(struct lenv* env) {
    for (size_t e = 0; e < env->len ; e++) {
        free(env->syms[e]);
        lval_free(env->vals[e]);
    }
    free(env->syms);
    free(env->vals);
    free(env);
}

bool lenv_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result) {
    if (!env) {
        return false;
    }
    const char* symbol = lval_as_sym(sym);
    for (size_t e = 0; e < env->len; e++) {
        if (strcmp(symbol, env->syms[e]) == 0) {
            lval_copy(result, env->vals[e]);
            return true;
        }
    }
    lval_mut_err(result, LERR_BAD_SYMBOL);
    return false;
}

bool lenv_put(struct lenv* env,
        const struct lval* sym, const struct lval* val) {
    if (!env) {
        return false;
    }
    const char* symbol = lval_as_sym(sym);
    if (!symbol) {
        return false;
    }
    /* Existing symbol? Override. */
    for (size_t e = 0; e < env->len; e++) {
        if (strcmp(symbol, env->syms[e]) == 0) {
            lval_clear(env->vals[e]);
            lval_copy(env->vals[e], val);
            return true;
        }
    }
    /* Not existing? Add symbol. */
    size_t len = ++env->len;
    env->syms = realloc(env->syms, sizeof(char*) * len);
    env->vals = realloc(env->vals, sizeof(struct lval*) * len);
    /* Copy symbol. */
    size_t slen = strlen(symbol);
    env->syms[len-1] = calloc(1, slen);
    strncpy(env->syms[len-1], symbol, slen);
    /* Copy value. */
    env->vals[len-1] = lval_alloc();
    lval_copy(env->vals[len-1], val);
    return true;
}

bool lenv_put_builtin(struct lenv* env,
        const char* symbol, const lbuiltin func) {
    if (!symbol || !func) {
        return false;
    }
    struct lval* sym = lval_alloc();
    lval_mut_sym(sym, symbol);
    struct lval* fun = lval_alloc();
    lval_mut_func(fun, func);
    bool s = lenv_put(env, sym, fun);
    lval_free(fun);
    lval_free(sym);
    return s;
}

bool lenv_default(struct lenv* env) {
    if (!env) {
        return false;
    }
    /* Arithmetic operators. */
    lenv_put_builtin(env, "+", lbuiltin_op_add);
    lenv_put_builtin(env, "-", lbuiltin_op_sub);
    lenv_put_builtin(env, "*", lbuiltin_op_mul);
    lenv_put_builtin(env, "/", lbuiltin_op_div);
    lenv_put_builtin(env, "%", lbuiltin_op_mod);
    lenv_put_builtin(env, "^", lbuiltin_op_pow);
    lenv_put_builtin(env, "!", lbuiltin_op_fac);
    /* List functions. */
    lenv_put_builtin(env, "head", lbuiltin_head);
    lenv_put_builtin(env, "tail", lbuiltin_tail);
    lenv_put_builtin(env, "init", lbuiltin_init);
    lenv_put_builtin(env, "cons", lbuiltin_cons);
    lenv_put_builtin(env, "len",  lbuiltin_len);
    lenv_put_builtin(env, "join", lbuiltin_join);
    lenv_put_builtin(env, "list", lbuiltin_list);
    lenv_put_builtin(env, "eval", lbuiltin_eval);
    return true;
}
