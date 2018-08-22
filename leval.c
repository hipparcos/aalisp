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

static bool leval_sexpr(struct lenv* env,
        const struct lval* v, struct lval* r) {
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
        if (!leval(env, child, x)) {
            lval_dup(r, x);
            lval_free(x);
            lval_free(child);
            lval_free(expr);
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
    /* First child is a symbol, it's an expression. */
    struct lval* child = lval_pop(expr, 0);
    struct lval* args = expr; // aliasing for clarity.
    struct lval* func = lval_alloc();
    /* Re-eval first arg; allow ((head {+ -}) 1 2). */
    if (!leval(env, child, func)) {
        lval_dup(r, func);
        lval_free(child);
        lval_free(func);
        lval_free(expr);
        return false;
    }
    lval_free(child);
    if (lval_type(func) == LVAL_FUNC) {
        lval_mut_nil(r);
        bool s = leval_expr(env, func, args, r);
        lval_free(func);
        lval_free(expr);
        leval_set_dot(env, r);
        return s;
    }
    lval_free(func);
    lval_free(expr);
    return true;
}

bool leval(struct lenv* env, const struct lval* v, struct lval* r) {
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
        return leval_sexpr(env, v, r);
    case LVAL_ERR:
        lval_dup(r, v);
        return false;
    default:
        lval_dup(r, v);
        return true;
    }
}

void print_error_marker(FILE* out, int indent, int col) {
    col--; // col starts at 0 in this function.
    for (int i = -indent; i < col; i++)
        fputc(' ', out);
    fputc('^', out);
    fputc('\n', out);
}

#define CLEANUP(tokens, ast, program) \
    do { \
    } while(0);

bool lisp_eval(struct lenv* env,
        const char* restrict input, struct lval* r, int prompt_len) {
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
        if (program && !leval(env, program, r)) {
            error = lerr_alloc();
            lerr_copy(error, lval_as_err(r));
            error = lerr_propagate(error, "eval error:");
            break;
        }
    } while (0); // Allow to break.
    /* Error handling. */
    bool s = true;
    if (error) {
        struct lerr* cause = lerr_cause(error);
        lerr_file_info(cause, "interactive", cause->line, cause->col);
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, cause->col);
            lerr_print_to(error, stderr);
        }
        lerr_free(error);
        s = false;
    }
    /* Cleanup. */
    if (tokens)  llex_free(tokens);
    if (ast)     last_free(ast);
    if (program) lval_free(program);
    return s;
}

void lisp_eval_from_string(struct lenv* env,
        const char* restrict input, int prompt_len) {
    struct lval* r = lval_alloc();
    if (lisp_eval(env, input, r, prompt_len)) {
        lval_println(r);
    } // Error already printed.
    lval_free(r);
}

/** read_file reads all content of filename at once.
 ** Caller is responsible for calling free on returned string. */
static char* read_file(FILE* input) {
    if (!input) {
        return NULL;
    }
    char* buffer = 0;
    long length;
    fseek(input, 0, SEEK_END);
    length = ftell (input);
    fseek(input, 0, SEEK_SET);
    buffer = malloc(length);
    if (buffer) {
        fread(buffer, 1, length, input);
    }
    return buffer;
}

void lisp_eval_from_file(struct lenv* env, FILE* input) {
    char* content = read_file(input);
    struct lval* r = lval_alloc();
    if (lisp_eval(env, content, r, 0)) {
        lval_println(r);
    } // Error already printed.
    lval_free(r);
    free(content);
}
