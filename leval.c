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

/** level_exec returns
 **   0 if success
 **  -1 if error
 **   1 if x generate an error
 **   2 if y generate an error */
static int leval_exec(const char* op, const struct lval* x, const struct lval* y, struct lval* r) {
    if (lval_type(x) == LVAL_ERR) {
        lval_copy(r, x);
        return -1;
    }
    if (lval_type(y) == LVAL_ERR) {
        lval_copy(r, y);
        return -1;
    }

    if (strcmp(op, "+") == 0) return lsym_exec(lbuiltin_op_add, x, y, r);
    if (strcmp(op, "-") == 0) return lsym_exec(lbuiltin_op_sub, x, y, r);
    if (strcmp(op, "*") == 0) return lsym_exec(lbuiltin_op_mul, x, y, r);
    if (strcmp(op, "/") == 0) return lsym_exec(lbuiltin_op_div, x, y, r);
    if (strcmp(op, "%") == 0) return lsym_exec(lbuiltin_op_mod, x, y, r);
    if (strcmp(op, "^") == 0) return lsym_exec(lbuiltin_op_pow, x, y, r);
    if (strcmp(op, "!") == 0) return lsym_exec(lbuiltin_op_fac, x, y, r);

    lval_mut_err(r, LERR_BAD_SYMBOL);
    return -1;
}

static bool leval(struct lval* v, struct lval* r);

static bool leval_expr(struct lval* v, struct lval* r) {
    struct lval* sym = lval_index(v, 0);
    const char* op = lval_as_sym(sym);
    switch (lval_len(v)) {
    case 1:  // 0 arguments.
        leval_exec(op, &lnil, &lnil, r);
        r->ast = sym->ast;
        lval_free(sym);
        break;
    case 2:  // 1 argument.
        {
        struct lval* x = lval_alloc();
        struct lval* arg = lval_index(v, 1);
        if (!leval(arg, x)) {
            lval_free(x);
            lval_free(arg);
            r->ast = x->ast;
            break;
        }
        int err = leval_exec(op, x, &lnil, r);
        switch (err) {
            case -1: r->ast = sym->ast; break;
            case  1: r->ast = arg->ast; break;
            default: break;
        }
        lval_free(x);
        lval_free(arg);
        }
        break;
    default: // multiple arguments
        {
        struct lval* x = lval_alloc();
        struct lval* arg = lval_index(v, 1);
        if (!leval(arg, x)) {
            lval_free(x);
            lval_free(arg);
            r->ast = x->ast;
            break;
        }
        lval_free(arg);
        struct lval* y = lval_alloc();
        size_t len = lval_len(v);
        for (size_t c = 2; c < len; c++) {
            /* Prepare arguments. */
            arg = lval_index(v, c);
            if (!leval(arg, y)) {
                lval_free(arg);
                r->ast = y->ast;
                break;
            }
            lval_free(arg);
            /* Execute. */
            int err = leval_exec(op, x, y, r);
            /* Error handling. */
            switch (err) {
                case -1: r->ast = sym->ast; break;
                case  1: r->ast = x->ast; break;
                case  2: r->ast = arg->ast; break;
                default: break;
            }
            /* Break on error. */
            if (lval_type(r) == LVAL_ERR) {
                break;
            }
            lval_dup(x, r);
        }
        lval_free(x);
        lval_free(y);
        }
    }
    lval_free(sym);
    return lval_type(r) != LVAL_ERR;
}

static bool leval_sexpr(struct lval* v, struct lval* r) {
    struct lval* child0 = lval_index(v, 0);
    /* First child is a symbol, it's an expression. */
    if (lval_type(child0) == LVAL_SYM) {
        lval_free(child0);
        return leval_expr(v, r);
    }
    /* Eval it as a list of sexpr; return the last value. */
    lval_free(child0);
    size_t len = lval_len(v);
    for (size_t c = 0; c < len; c++) {
        struct lval* child = lval_index(v, c);
        leval(child, r);
        lval_free(child);
        if (lval_type(r) == LVAL_ERR) {
            return false;
        }
    }
    return true;
}

static bool leval(struct lval* v, struct lval* r) {
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
