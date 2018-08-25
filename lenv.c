#include "lenv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"
#include "lfunc.h"
#include "lbuiltin.h"
#include "leval.h"
#include "generic/avl.h"

struct env_payload {
    char* key;
    struct lval* val;
};

static struct env_payload* env_payload_alloc(const char* key, const struct lval* val) {
    struct env_payload* pl = calloc(1, sizeof(struct env_payload));
    size_t len = strlen(key);
    pl->key = calloc(len + 1, 1);
    strncpy(pl->key, key, len);
    pl->val = lval_alloc();
    lval_copy(pl->val, val);
    return pl;
}

static void* env_payload_copy(const void* srcv) {
    struct env_payload* src = (struct env_payload*)srcv;
    struct env_payload* dest = env_payload_alloc(src->key, src->val);
    return (void*)dest;
}

static void env_payload_free(void* datav) {
    struct env_payload* data = (struct env_payload*)datav;
    if (data->key) {
        free(data->key);
    }
    if (data->val) {
        lval_free(data->val);
    }
    free(data);
}

static int env_payload_cmp(const void* leftv, const void* rightv) {
    struct env_payload* left = (struct env_payload*)leftv;
    struct env_payload* right = (struct env_payload*)rightv;
    return strcmp(left->key, right->key);
}

static const char* env_payload_key(const void* datav) {
    struct env_payload* data = (struct env_payload*)datav;
    return data->key;
}

/** lenv associates a symbol descriptor with a string. */
struct lenv {
    /** lenv.par is the parent environment. */
    struct lenv* par;
    /** lenv.len is the number of symbol in this env (not its parent). */
    size_t len;
    /** lenv.tree is the AVL tree containing defined symbols. */
    struct avl_node* tree;
};

struct lenv* lenv_alloc(void) {
    struct lenv* env = calloc(1, sizeof(struct lenv));
    env->par = NULL;
    env->len = 0;
    env->tree = NULL;
    return env;
}

static void lenv_clear(struct lenv* env) {
    env->len = 0;
    avl_free(env->tree, env_payload_free);
    env->tree = NULL;
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
    dest->par = src->par;
    dest->len = src->len;
    dest->tree = avl_duplicate(src->tree, env_payload_copy);
    return true;
}

#define CHECK(cond) if (!(cond)) return false;
bool lenv_are_equal(const struct lenv* left, const struct lenv* right) {
    if (left == right) {
        return true;
    }
    CHECK(left && right);
    /* Total length. */
    size_t lenl = lenv_len(left);
    size_t lenr = lenv_len(right);
    CHECK(lenl == lenr);
    /* Check all symbols. */
    const struct lenv* env = left;
    struct lval* sym = lval_alloc();
    do {
        size_t len = 0;
        const char** syms = avl_keys(env->tree, env_payload_key, &len);
        for (size_t k = 0; k < len; k++) {
            lval_mut_sym(sym, syms[k]);
            if (!lenv_lookup(right, sym, NULL)) {
                lval_free(sym);
                return false;
            }
        }
        free(syms);
    } while ((env = env->par));
    lval_free(sym);
    return true;
}

size_t lenv_len(const struct lenv* env) {
    if (!env) {
        return 0;
    }
    size_t len = env->len;
    while ((env = env->par)) { len += env->len; }
    return len;
}

#define indent(indent, width, out) \
    do { \
        size_t spaces = indent * 2; \
        while (spaces--) { \
            fputc(' ', out); \
            width++; \
        } \
    } while(0);
void lenv_print_to(const struct lenv* env, FILE* out) {
    size_t indent = 0;
    size_t wrap = 80;
    while (env) {
        size_t len = 0;
        const char** syms = avl_keys(env->tree, env_payload_key, &len);
        size_t width = 0;
        indent(indent, width, out);
        width += fprintf(out, "lenv(%ld){", len);
        if (syms) {
            for (size_t k = 0; k < len; k++) {
                if (k) {
                    fputc(',', out);
                    width++;
                }
                size_t len = strlen(syms[k]);
                if (width + len >= wrap) {
                    fputc('\n', out);
                    width = 0;
                    indent(indent+1, width, out);
                }
                fputs(syms[k], out);
                width += len;
            }
            free(syms);
        }
        fputs("}\n", out);
        env = env->par;
        indent++;
    }
}

bool lenv_set_parent(struct lenv* env, struct lenv* par) {
    if (!env) {
        return false;
    }
    env->par = par;
    return true;
}

static bool lenv_local_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result) {
    const char* symbol = lval_as_sym(sym);
    if (!symbol) {
        struct lerr* err = lerr_throw(LERR_BAD_SYMBOL, "lookup for nil");
        lval_mut_err_ptr(result, err);
        return false;
    }
    struct env_payload symbolpl = {.key= (char*)symbol};
    const struct env_payload* payload = avl_lookup(env->tree, env_payload_cmp, &symbolpl);
    /* Symbol found. */
    if (payload) {
        lval_copy(result, payload->val);
        /* Don't return lval_copy return value because result can be NULL,
         * thus lval_copy fails. */
        return true;
    }
    /* Fail. */
    struct lerr* err = lerr_throw(LERR_BAD_SYMBOL,
            "symbol %s not defined in environment", symbol);
    lval_mut_err_ptr(result, err);
    return false;
}

bool lenv_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result) {
    if (!env) {
        return false;
    }
    return lenv_local_lookup(env, sym, result)
        || lenv_lookup(env->par, sym, result);
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
    /* Insert into AVL. */
    bool insertion = false;
    struct env_payload* payload = env_payload_alloc(symbol, val);
    /* Special case for function definition. */
    if (lval_type(val) == LVAL_FUNC) {
        struct lfunc* fun = lval_as_func(payload->val);
        lfunc_set_symbol(fun, symbol);
    }
    struct avl_node* node = avl_alloc(payload);
    env->tree = avl_insert(env->tree, node, env_payload_cmp, env_payload_free, &insertion);
    if (insertion) {
        env->len++;
    }
    return !avl_is_nil(env->tree);
}

/* Caution: call this function with statically allocated symbol string only. */
static bool lenv_put_builtin(struct lenv* env,
        const char* symbol, const struct lfunc* func) {
    if (!symbol || !func) {
        return false;
    }
    struct lval* sym = lval_alloc();
    lval_mut_sym(sym, symbol);
    struct lval* fun = lval_alloc();
    lval_mut_func(fun, func);
    struct lfunc* func_ptr = lval_as_func(fun);
    lfunc_set_symbol(func_ptr, symbol);
    bool s = lenv_put(env, sym, fun);
    lval_free(fun);
    lval_free(sym);
    return s;
}

static bool lenv_put_lval(struct lenv* env,
        const char* symbol, struct lval* val) {
    if (!symbol) {
        return false;
    }
    struct lval* lval = val;
    if (!val) {
        lval = lval_alloc();
    }
    struct lval* sym = lval_alloc();
    lval_mut_sym(sym, symbol);
    bool s = lenv_put(env, sym, lval);
    lval_free(sym);
    if (!val) {
        lval_free(lval);
    }
    return s;
}

bool lenv_override(struct lenv* env,
        const struct lval* sym, const struct lval* val) {
    if (!env) {
        return false;
    }
    if (lenv_local_lookup(env, sym, NULL)) {
        return lenv_put(env, sym, val);
    }
    return lenv_override(env->par, sym, val);
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
    /* Boolean operators. */
    lenv_put_builtin(env, "==", &lbuiltin_op_eq);
    lenv_put_builtin(env, "!=", &lbuiltin_op_neq);
    lenv_put_builtin(env, ">", &lbuiltin_op_gt);
    lenv_put_builtin(env, ">=", &lbuiltin_op_gte);
    lenv_put_builtin(env, "<", &lbuiltin_op_lt);
    lenv_put_builtin(env, "<=", &lbuiltin_op_lte);
    lenv_put_builtin(env, "and", &lbuiltin_op_and);
    lenv_put_builtin(env, "or", &lbuiltin_op_or);
    lenv_put_builtin(env, "not", &lbuiltin_op_not);
    /* Control flow functions. */
    lenv_put_builtin(env, "if", &lbuiltin_if);
    lenv_put_builtin(env, "loop", &lbuiltin_loop);
    /* List manipulation functions. */
    lenv_put_builtin(env, "head", &lbuiltin_head);
    lenv_put_builtin(env, "tail", &lbuiltin_tail);
    lenv_put_builtin(env, "init", &lbuiltin_init);
    lenv_put_builtin(env, "last", &lbuiltin_last);
    lenv_put_builtin(env, "index", &lbuiltin_index);
    lenv_put_builtin(env, "!!", &lbuiltin_index);
    lenv_put_builtin(env, "drop", &lbuiltin_drop);
    lenv_put_builtin(env, "cons", &lbuiltin_cons);
    lenv_put_builtin(env, "len",  &lbuiltin_len);
    lenv_put_builtin(env, "join", &lbuiltin_join);
    lenv_put_builtin(env, "++", &lbuiltin_join);
    lenv_put_builtin(env, "list", &lbuiltin_list);
    lenv_put_builtin(env, "eval", &lbuiltin_eval);
    lenv_put_builtin(env, "map", &lbuiltin_map);
    /* Environment manipulation functions. */
    lenv_put_builtin(env, "def", &lbuiltin_def);
    lenv_put_builtin(env, "ovr", &lbuiltin_override);
    lenv_put_builtin(env, "put", &lbuiltin_put);
    lenv_put_builtin(env, "=",   &lbuiltin_put);
    lenv_put_builtin(env, "fun", &lbuiltin_fun);
    lenv_put_builtin(env, "load", &lbuiltin_load);
    /* Functions manipulation functions. */
    lenv_put_builtin(env, "lambda", &lbuiltin_lambda);
    lenv_put_builtin(env, "\\",     &lbuiltin_lambda);
    lenv_put_builtin(env, "pack",    &lbuiltin_pack);
    lenv_put_builtin(env, "uncurry", &lbuiltin_pack);
    lenv_put_builtin(env, "unpack",  &lbuiltin_unpack);
    lenv_put_builtin(env, "curry",   &lbuiltin_unpack);
    lenv_put_builtin(env, "partial", &lbuiltin_partial);
    lenv_put_builtin(env, "::", &lbuiltin_partial);
    /* IO functions. */
    lenv_put_builtin(env, "print", &lbuiltin_print);
    /* Error functions. */
    lenv_put_builtin(env, "error", &lbuiltin_error);
    /* Environment variable. */
    lenv_put_lval(env, ".", NULL);
    lenv_put_lval(env, "nil", (struct lval*)&lnil);
    struct lval* boolean = lval_alloc();
    lval_mut_bool(boolean, true);
    lenv_put_lval(env, "true", boolean);
    lval_mut_bool(boolean, false);
    lenv_put_lval(env, "false", boolean);
    lval_free(boolean);
    return true;
}
