#include "lenv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "lfunc.h"
#include "lbuiltin.h"

/** lenv associates a symbol descriptor with a string. */
struct lenv {
    char**  syms;
    struct lval** vals;
    size_t len;
    struct lenv* par;
};

struct lenv* lenv_alloc(void) {
    struct lenv* env = calloc(1, sizeof(struct lenv));
    env->syms = NULL;
    env->vals = NULL;
    env->len = 0;
    env->par = NULL;
    return env;
}

static void lenv_clear(struct lenv* env) {
    for (size_t e = 0; e < env->len ; e++) {
        free(env->syms[e]);
        lval_free(env->vals[e]);
    }
    free(env->syms);
    env->syms = NULL;
    free(env->vals);
    env->vals = NULL;
    env->len = 0;
}

void lenv_free(struct lenv* env) {
    if (!env) {
        return;
    }
    lenv_clear(env);
    free(env);
}

bool lenv_copy(struct lenv* dest, const struct lenv* src) {
    if (!dest || !src) {
        return false;
    }
    lenv_clear(dest);
    dest->syms = calloc(src->len, sizeof(char*));
    dest->vals = calloc(src->len, sizeof(struct lval*));
    for (size_t e = 0; e < src->len; e++) {
        /* Symbols. */
        size_t len = strlen(src->syms[e]);
        dest->syms[e] = calloc(len + 1, 1);
        strncpy(dest->syms[e], src->syms[e], len);
        /* Values. */
        dest->vals[e] = lval_alloc();
        lval_dup(dest->vals[e], src->vals[e]);
    }
    dest->len = src->len;
    dest->par = src->par;
    return true;
}

#define CHECK(cond) if (!(cond)) return false;
bool lenv_are_equal(const struct lenv* left, const struct lenv* right) {
    if (left == right) {
        return true;
    }
    CHECK(left && right);
    /* Total length. */
    struct lenv* par = NULL;
    size_t lenl = left->len;
    while ((par = left->par)) { lenl += par->len; }
    size_t lenr = right->len;
    while ((par = right->par)) { lenr += par->len; }
    CHECK(lenl == lenr);
    /* Check all symbols. */
    const struct lenv* env = left;
    struct lval* sym = lval_alloc();
    do {
        for (size_t e = 0; e < env->len; e++) {
            lval_mut_sym(sym, env->syms[e]);
            if (!lenv_lookup(right, sym, NULL)) {
                lval_free(sym);
                return false;
            }
        }
    } while ((env = env->par));
    lval_free(sym);
    return true;
}

bool lenv_set_parent(struct lenv* env, struct lenv* par) {
    if (!env || !par) {
        return false;
    }
    env->par = par;
    return true;
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
    if (env->par) {
        return lenv_lookup(env->par, sym, result);
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
        const char* symbol, const struct lfunc* func) {
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

bool lenv_def(struct lenv* env,
        const struct lval* sym, const struct lval* val) {
    if (!env) {
        return false;
    }
    while (env->par) { env = env->par; }
    return lenv_put(env, sym, val);
}

bool lenv_default(struct lenv* env) {
    if (!env) {
        return false;
    }
    /* Arithmetic operators. */
    lenv_put_builtin(env, "+", &lbuiltin_op_add);
    lenv_put_builtin(env, "-", &lbuiltin_op_sub);
    lenv_put_builtin(env, "*", &lbuiltin_op_mul);
    lenv_put_builtin(env, "/", &lbuiltin_op_div);
    lenv_put_builtin(env, "%", &lbuiltin_op_mod);
    lenv_put_builtin(env, "^", &lbuiltin_op_pow);
    lenv_put_builtin(env, "!", &lbuiltin_op_fac);
    /* List manipulation functions. */
    lenv_put_builtin(env, "head", &lbuiltin_head);
    lenv_put_builtin(env, "tail", &lbuiltin_tail);
    lenv_put_builtin(env, "init", &lbuiltin_init);
    lenv_put_builtin(env, "last", &lbuiltin_last);
    lenv_put_builtin(env, "cons", &lbuiltin_cons);
    lenv_put_builtin(env, "len",  &lbuiltin_len);
    lenv_put_builtin(env, "join", &lbuiltin_join);
    lenv_put_builtin(env, "list", &lbuiltin_list);
    lenv_put_builtin(env, "eval", &lbuiltin_eval);
    /* Environment manipulation functions. */
    lenv_put_builtin(env, "def", &lbuiltin_def);
    lenv_put_builtin(env, "put", &lbuiltin_put);
    lenv_put_builtin(env, "=",   &lbuiltin_put);
    lenv_put_builtin(env, "fun", &lbuiltin_fun);
    lenv_put_builtin(env, "lambda", &lbuiltin_lambda);
    lenv_put_builtin(env, "\\",     &lbuiltin_lambda);
    return true;
}
