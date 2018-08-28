#include "leval.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "llexer.h"
#include "lparser.h"
#include "lmut.h"
#include "lval.h"
#include "lenv.h"
#include "lfunc.h"
#include "lbuiltin.h"

static bool leval_lval(struct lenv* env, const struct lval* v, struct lval* r, bool exec);

static bool leval_expr(
        struct lenv* env, const struct lval* func, const struct lval* args, struct lval* r) {
    /* Execute expression. */
    int err = lfunc_exec(lval_as_func(func), env, args, r);
    /* Error handling. */
    if (err != 0) {
        if (err == -1) {
            r->ast = func->ast;
        } else {
            /* Set r->ast to the node returning an error. */
            struct lval* child = lval_alloc();
            lval_index(args, err-1, child);
            r->ast = child->ast;
            lval_free(child);
        }
        struct lerr* cause = lerr_cause(lval_as_err(r));
        if (r->ast) {
            lerr_set_location(cause, r->ast->line, r->ast->col);
        }
    }
    return lval_type(r) != LVAL_ERR;
}

/** leval_set_dot sets the special dot variable (last computed value). */
static void leval_set_dot(struct lenv* env, const struct lval* r) {
    struct lval* dot = lval_alloc();
    lval_mut_sym(dot, ".");
    lenv_def(env, dot, r);
    lval_free(dot);
}

/** leval_sexpr evaluates an S-Expression.
 ** exec tells if the S-Expression should be evaluated like an expression. */
static bool leval_sexpr(struct lenv* env,
        const struct lval* v, struct lval* r, bool exec) {
    /* Empty sexpr. */
    size_t len = lval_len(v);
    if (len == 0) {
        lval_mut_nil(r);
        return true;
    }
    /* Evaluates all children. */
    struct lval* expr = lval_alloc();
    lval_mut_sexpr(expr);
    for (size_t c = 0; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(v, c, child);
        struct lval* x = lval_alloc();
        if (!leval_lval(env, child, x, true)) {
            lval_dup(r, x);
            lval_free(x);
            lval_free(child);
            lval_free(expr);
            struct lerr* cause = lerr_cause(lval_as_err(r));
            if (r->ast) {
                lerr_set_location(cause, r->ast->line, r->ast->col);
            }
            return false;
        }
        lval_push(expr, x);
        /* Result of last S-Expression. */
        if (c == len-1) {
            /* r = last argument value */
            lval_mut_nil(r);
            lval_dup(r, x);
            leval_set_dot(env, r);
        }
        lval_free(x);
        lval_free(child);
    }
    if (!exec) {
        lval_free(expr);
        return true;
    }
    /* First child is a symbol, it's an expression. */
    bool s = true;
    struct lval* child = lval_pop(expr, 0);
    struct lval* args = expr; // aliasing for clarity.
    if (lval_type(child) == LVAL_FUNC) {
        lval_mut_nil(r);
        s = leval_expr(env, child, args, r);
        leval_set_dot(env, r);
    }
    lval_free(child);
    lval_free(expr);
    return s;
}

/** leval_lval does the proper action depending of the type of v.
 ** exec tells if the S-Expression should be evaluated like an expression. */
static bool leval_lval(struct lenv* env, const struct lval* v, struct lval* r, bool exec) {
    if (!v) {
        struct lerr* err = lerr_throw(LERR_EVAL,
                "the impossible happens, NULL pointer received");
        lval_mut_err_ptr(r, err);
        return false;
    }
    switch (lval_type(v)) {
    case LVAL_SYM:
        {
        bool s = lenv_lookup(env, v, r);
        r->ast = v->ast;
        return s;
        }
    case LVAL_SEXPR:
        return leval_sexpr(env, v, r, exec);
    case LVAL_ERR:
        lval_dup(r, v);
        return false;
    default:
        lval_dup(r, v);
        return true;
    }
}

bool leval(struct lenv* env, const struct lval* v, struct lval* r) {
    return leval_lval(env, v, r, true);
}

struct lerr* leval_from_string(struct lenv* env,
        const char* restrict input, struct lval* r) {
    struct ltok* tokens = NULL;
    struct last* ast = NULL;
    struct lval* program = NULL;
    struct lerr* error = NULL;
    do {
        /* Lex input. */
        tokens = lisp_lex_surround(input, &error);
        if (error) {
            error = lerr_propagate(error, "lexing error:");
            lval_mut_err_code(r, LERR_EVAL);
            break;
        }
        /* Parse input. */
        ast = lisp_parse(tokens, &error);
        if (error) {
            error = lerr_propagate(error, "parsing error:");
            lval_mut_err_code(r, LERR_EVAL);
            break;
        }
        /* Mutate the AST. */
        program = lisp_mut(ast, &error);
        if (error) {
            error = lerr_propagate(error, "mutation error:");
            lval_mut_err_code(r, LERR_EVAL);
            break;
        }
        /* Evaluate program. */
        if (program && !leval_lval(env, program, r, false)) {
            error = lerr_alloc();
            lerr_copy(error, lval_as_err(r));
            error = lerr_propagate(error, "eval error:");
            break;
        }
    } while (0); // Allow to break.
    /* Cleanup. */
    if (tokens)  llex_free(tokens);
    if (ast)     last_free(ast);
    if (program) lval_free(program);
    return error;
}

struct lerr* lisp_eval_from_string(struct lenv* env,
        const char* restrict input) {
    struct lval* r = lval_alloc();
    struct lerr* err = leval_from_string(env, input, r);
    if (!err) {
        lval_println(r);
    }
    lval_free(r);
    return err;
}

/** read_file reads all content of filename at once.
 ** Caller is responsible for calling free on returned string. */
static char* read_file(FILE* input) {
    if (!input) {
        return NULL;
    }
    fseek(input, 0L, SEEK_END);
    long length = ftell(input);
    if (length < 0) {
        return NULL;
    }
    fseek(input, 0L, SEEK_SET);
    char* buffer = malloc(length+1);
    if (buffer) {
        if (fread(buffer, 1, length, input) < 0) {
            free(buffer);
            return NULL;
        }
        buffer[length] = '\0';
    }
    return buffer;
}

struct lerr* leval_from_file(struct lenv* env, FILE* input, struct lval* r) {
    char* content = read_file(input);
    struct lerr* err = leval_from_string(env, content, r);
    free(content);
    return err;
}

struct lerr* lisp_eval_from_file(struct lenv* env, FILE* input) {
    char* content = read_file(input);
    struct lerr* err = lisp_eval_from_string(env, content);
    free(content);
    return err;
}
