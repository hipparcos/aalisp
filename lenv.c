#include "lenv.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lval.h"
#include "lfunc.h"
#include "lbuiltin.h"
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
    if (!symbol) {
        lval_mut_err(result, LERR_BAD_SYMBOL);
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
    /* Lookup into parent env. */
    if (env->par) {
        return lenv_lookup(env->par, sym, result);
    }
    /* Fail. */
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
    /* Insert into AVL. */
    bool insertion = false;
    struct env_payload* payload = env_payload_alloc(symbol, val);
    struct avl_node* node = avl_alloc(payload);
    env->tree = avl_insert(env->tree, node, env_payload_cmp, env_payload_free, &insertion);
    if (insertion) {
        env->len++;
    }
    return !avl_is_nil(env->tree);
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
    /* IO functions. */
    lenv_put_builtin(env, "print", &lbuiltin_print);
    return true;
}
