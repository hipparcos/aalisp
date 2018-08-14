#include "leval.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "lbuiltin.h"
#include "lval.h"
#include "lsym.h"
#include "llexer.h"
#include "lparser.h"
#include "lmut.h"

struct lsym_table lbuiltins[] = {
    /* Arithmetic operators. */
    {"+", &lbuiltin_op_add},
    {"-", &lbuiltin_op_sub_unary},
    {"-", &lbuiltin_op_sub},
    {"*", &lbuiltin_op_mul},
    {"/", &lbuiltin_op_div},
    {"%", &lbuiltin_op_mod},
    {"^", &lbuiltin_op_pow},
    {"!", &lbuiltin_op_fac},
    /* List functions. */
    {"head", &lbuiltin_head},
    {"tail", &lbuiltin_tail},
    {"init", &lbuiltin_init},
    {"len",  &lbuiltin_len},
    {"join", &lbuiltin_join},
    {"list", &lbuiltin_list},
    {"eval", &lbuiltin_eval},
    /* ... */
    {NULL, NULL},
};

/** level_exec returns
 **   0 if success
 **  -1 if error
 **   1 if acc generate an error
 **   2 if x generate an error */
static int leval_exec(const char* op, struct lval* acc, const struct lval* x, size_t operands) {
    if (lval_type(acc) == LVAL_ERR) {
        return -1;
    }
    if (lval_type(x) == LVAL_ERR) {
        lval_copy(acc, x);
        return -1;
    }

    const struct lsym* descriptor = NULL;
    if (NULL != (descriptor = lsym_lookup(lbuiltins, op, operands))) {
        return lsym_exec(descriptor, acc, x);
    }

    lval_mut_err(acc, LERR_BAD_SYMBOL);
    return -1;
}

static bool leval_expr(const struct lval* v, struct lval* r) {
    struct lval* sym = lval_alloc();
    /* Empty sexpr. */
    if (!lval_index(v, 0, sym)) {
        lval_free(sym);
        return true;
    }
    const char* op = lval_as_sym(sym);
    size_t c = 1; /* First argument is at index 1. */
    size_t len = lval_len(v);
    do {
        struct lval* x = lval_alloc();
        struct lval* child = lval_alloc();
        lval_index(v, c, child);
        if (!child) {
            child = lval_alloc(); /* nil */
        };
        if (!leval(child, x)) {
            lval_dup(r, x);
            r->ast = x->ast;
            lval_free(x);
            lval_free(child);
            break;
        }
        lval_free(child);
        int err = leval_exec(op, r, x, len - 1);
        /* Error handling. */
        switch (err) {
            case -1: r->ast = sym->ast; break;
            case  1: r->ast = x->ast; break;
            case  2: r->ast = x->ast; break;
            default: break;
        }
        /* Break on error. */
        if (lval_type(r) == LVAL_ERR) {
            break;
        }
        lval_free(x);
    } while (++c < len);
    lval_free(sym);
    return lval_type(r) != LVAL_ERR;
}

static bool leval_sexpr(const struct lval* v, struct lval* r) {
    struct lval* child0 = lval_alloc();
    lval_index(v, 0, child0);
    /* First child is a symbol, it's an expression. */
    if (lval_type(child0) == LVAL_SYM) {
        lval_free(child0);
        return leval_expr(v, r);
    }
    /* Eval it as a list of sexpr; return the last value. */
    lval_free(child0);
    size_t len = lval_len(v);
    for (size_t c = 0; c < len; c++) {
        lval_mut_nil(r);
        struct lval* child = lval_alloc();
        lval_index(v, c, child);
        leval(child, r);
        lval_free(child);
        if (lval_type(r) == LVAL_ERR) {
            return false;
        }
    }
    return true;
}

bool leval(const struct lval* v, struct lval* r) {
    if (!v) {
        lval_mut_err(r, LERR_EVAL);
        return false;
    }
    switch (lval_type(v)) {
    case LVAL_SEXPR:
        return leval_sexpr(v, r);
    case LVAL_ERR:
        lval_dup(r, v);
        return false;
    default:
        lval_dup(r, v);
        r->ast = v->ast;
        return true;
    }
}

static void print_error_marker(FILE* out, int indent, int col) {
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

bool lisp_eval(const char* restrict input, struct lval* r, int prompt_len) {
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
    if (program && !leval(program, r)) {
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

void lisp_eval_from_string(const char* restrict input, int prompt_len) {
    struct lval* r = lval_alloc();
    if (lisp_eval(input, r, prompt_len)) {
        lval_println(r);
    } // Error already printed.
    lval_free(r);
}
