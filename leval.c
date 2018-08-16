#include "leval.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "lbuiltin.h"
#include "lval.h"
#include "lenv.h"
#include "llexer.h"
#include "lparser.h"
#include "lmut.h"

/** level_exec returns
 **   0 if success
 **  -1 if error
 **   n if nth argument generate an error */
static int leval_exec(const struct lval* func, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    lbuiltin f = lval_as_func(func);
    if (!f) {
        return -1;
    }
    return f(env, args, acc);
}

static bool leval_expr(struct lenv* env, struct lval* expr, struct lval* r) {
    /* First child is the symbol. */
    struct lval* func = lval_pop(expr, 0);
    struct lval* args = expr; // aliasing for clarity.
    /* Execute expression. */
    int err = leval_exec(func, env, args, r);
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
    lval_free(func);
    return lval_type(r) != LVAL_ERR;
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
        if (c == len-1) {
            /* r = last argument value */
            lval_mut_nil(r);
            lval_dup(r, x);
        }
        lval_free(x);
        lval_free(child);
    }
    /* First child is a symbol, it's an expression. */
    struct lval* child = lval_alloc();
    lval_index(expr, 0, child);
    if (lval_type(child) == LVAL_FUNC) {
        lval_free(child);
        lval_mut_nil(r);
        bool s = leval_expr(env, expr, r);
        lval_free(expr);
        return s;
    }
    lval_free(child);
    lval_free(expr);
    return true;
}

bool leval(struct lenv* env, const struct lval* v, struct lval* r) {
    if (!v) {
        lval_mut_err(r, LERR_EVAL);
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
        if (tokens)  llex_free(tokens); \
        if (ast)     last_free(ast); \
        if (program) lval_free(program); \
    } while(0);

bool lisp_eval(struct lenv* env,
        const char* restrict input, struct lval* r, int prompt_len) {
    /* Lex input. */
    struct ltok *tokens = NULL, *lexer_error = NULL;
    tokens = lisp_lex_surround(input, &lexer_error);
    if (lexer_error != NULL) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, lexer_error->col);
            fprintf(stderr, "<interactive>:%d:%d: lexing error: %s.\n",
                    lexer_error->line, lexer_error->col, lexer_error->content);
        }
        lval_mut_err(r, LERR_EVAL);
        CLEANUP(tokens, NULL, NULL);
        return false;
    }
    /* Parse input. */
    struct last *ast, *parser_error = NULL;
    ast = lisp_parse(tokens, &parser_error);
    if (parser_error != NULL) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, parser_error->col);
            fprintf(stderr, "<interactive>:%d:%d: parsing error: %s.\n",
                    parser_error->line, parser_error->col, parser_error->content);
        }
        lval_mut_err(r, LERR_EVAL);
        CLEANUP(tokens, ast, NULL);
        return false;
    }
    /* Mutate the AST. */
    struct lval* program = NULL;
    struct last* mut_error = NULL;
    program = lisp_mut(ast, &mut_error);
    if (mut_error != NULL) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, mut_error->col);
            fprintf(stderr, "<interactive>:%d:%d: mutation error: %s.\n",
                    mut_error->line, mut_error->col, mut_error->content);
        }
        lval_mut_err(r, LERR_EVAL);
        CLEANUP(tokens, ast, program);
        return false;
    }
    /* Evaluate program. */
    if (program && !leval(env, program, r)) {
        if (prompt_len >= 0) {
            int line = 0, col = 0;
            if (r->ast) {
                line = r->ast->line;
                col  = r->ast->col;
            }
            print_error_marker(stderr, prompt_len, col);
            size_t len = lval_printlen(r);
            char* err = calloc(1, len);
            lval_as_str(r, err, len);
            fprintf(stderr, "<interactive>:%d:%d: evaluation error: %s.\n",
                    line, col, err);
            free(err);
        }
        CLEANUP(tokens, ast, program);
        return false;
    }
    CLEANUP(tokens, ast, program);
    return true;
}

void lisp_eval_from_string(struct lenv* env,
        const char* restrict input, int prompt_len) {
    struct lval* r = lval_alloc();
    if (lisp_eval(env, input, r, prompt_len)) {
        lval_println(r);
    } // Error already printed.
    lval_free(r);
}
