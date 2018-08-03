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

/** level_exec returns
 **   0 if success
 **  -1 if error
 **   1 if x generate an error
 **   2 if y generate an error */
static int leval_exec(char* op, const struct lval* x, const struct lval* y, struct lval* r) {
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

static bool leval_num(struct last* ast, struct lval* r, struct last** error) {
    (void)error;
    errno = 0;
    long n = strtol(ast->content, NULL, 10);
    if (errno == ERANGE) {
        /* Switch to bignum. */
        mpz_t bignum;
        mpz_init_set_str(bignum, ast->content, 10);
        lval_mut_bignum(r, bignum);
        mpz_clear(bignum);
        return true;
    }
    lval_mut_num(r, n);
    return true;
}

static bool leval_dbl(struct last* ast, struct lval* r, struct last** error) {
    errno = 0;
    double d = strtod(ast->content, NULL);
    if (errno == ERANGE) {
        lval_mut_err(r, LERR_BAD_OPERAND);
        *error = ast;
        return false;
    }
    lval_mut_dbl(r, d);
    return true;
}

static bool leval_ast(struct last* ast, struct lval* r, struct last** error);

static bool leval_expr(struct last* ast, struct lval* r, struct last** error) {
    if (ast->children[0]->tag == LTAG_SEXPR) {
        return leval_ast(ast->children[0], r, error);
    }
    int ret = 0;
    char* op = ast->children[0]->content;
    struct lval *x = NULL, *y = NULL;
    switch (ast->childrenc - 1) {
    case 0: // no arguments.
        if (leval_exec(op, &lnil, &lnil, r) != 0) {
            *error = ast;
            return false;
        }
        break;
    case 1: // 1 argument.
        x = lval_alloc();
        if (!leval_ast(ast->children[1], x, error)) {
            lval_clear(r);
            lval_copy(r, x);
            lval_free(x);
            return false;
        }
        if ((ret = leval_exec(op, x, &lnil, r)) != 0) {
            if (ret == 1) {
                *error = ast->children[1];
            } else {
                *error = ast;
            }
            lval_free(x);
            return false;
        }
        lval_free(x);
        break;
    default: // >= 2
        x = lval_alloc();
        if (!leval_ast(ast->children[1], x, error)) {
            lval_clear(r);
            lval_copy(r, x);
            lval_free(x);
            return false;
        }
        lval_copy(r, x);
        for (size_t i = 2; i < ast->childrenc; i++) {
            y = lval_alloc();
            if (!leval_ast(ast->children[i], y, error)) {
                lval_clear(r);
                lval_copy(r, y);
                lval_free(y);
                lval_free(x);
                return false;
            }
            if ((ret = leval_exec(op, r, y, r)) != 0) {
                if (ret == 1) {
                    *error = ast->children[i-1];
                } else if (ret == 2) {
                    *error = ast->children[i];
                } else {
                    *error = ast;
                }
                lval_free(y);
                lval_free(x);
                return false;
            }
            lval_free(y);
        }
        lval_free(x);
        break;
    }
    return true;
}

static bool leval_ast(struct last* ast, struct lval* r, struct last** error) {
    if (!ast) {
        lval_mut_err(r, LERR_EVAL);
        return false;
    }
    switch (ast->tag) {
    case LTAG_PROG:
        for (size_t s = 0; s < ast->childrenc; s++) {
            if (!leval_ast(ast->children[s], r, error)) {
                return false;
            }
        }
        return true;
    case LTAG_NUM:
        return leval_num(ast, r, error);
    case LTAG_DBL:
        return leval_dbl(ast, r, error);
    case LTAG_SEXPR:
        return leval_expr(ast->children[0], r, error);
    case LTAG_EXPR:
        return leval_expr(ast, r, error);
    case LTAG_STR:
        lval_mut_str(r, ast->content);
        return true;
    case LTAG_SYM:
        lval_mut_sym(r, ast->content);
        return true;
    default: break;
    }
    lval_mut_err(r, LERR_UNKNOWN);
    *error = ast;
    return false;
}

/* static void pretty_print_error(FILE* out, const char* input, int col) { */
/*     int ctx = 10; */
/*     /1* In this function col starts at 0. *1/ */
/*     col = col - 1; */
/*     size_t starti = (col < ctx) ? 0 : col - ctx; */
/*     const char* start = input + starti; */
/*     size_t endi = col + ctx; */
/*     size_t inputlen = strlen(input); */
/*     if (endi > inputlen) { */
/*         endi = inputlen; */
/*     } */
/*     size_t len = endi - starti; */
/*     char* buf = calloc(1, len + 1); */
/*     strncpy(buf, start, len); */
/*     fputs(buf, out); */
/*     fputc('\n', out); */
/*     for (int i = starti; i < col; i++) */
/*         fputc(' ', out); */
/*     fputc('^', out); */
/*     fputc('\n', out); */
/*     free(buf); */
/* } */

static void print_error_marker(FILE* out, int indent, int col) {
    col--; // col starts at 0 in this function.
    for (int i = -indent; i < col; i++)
        fputc(' ', out);
    fputc('^', out);
    fputc('\n', out);
}

bool lisp_eval(const char* restrict input, struct lval* r, int prompt_len) {
    /* Lex input. */
    struct ltok *tokens = NULL, *lexer_error = NULL;
    tokens = lisp_lex_surround(input, &lexer_error);
    if (lexer_error != NULL) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, lexer_error->col);
            fprintf(stderr, "<stdin>:%d:%d: lexing error: %s.\n",
                    lexer_error->line, lexer_error->col, lexer_error->content);
        }
        llex_free(tokens);
        lval_mut_err(r, LERR_EVAL);
        return false;
    }
    /* Parse input. */
    struct last *ast, *parser_error = NULL;
    ast = lisp_parse(tokens, &parser_error);
    if (parser_error != NULL) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, parser_error->col);
            fprintf(stderr, "<stdin>:%d:%d: parsing error: %s.\n",
                    parser_error->line, parser_error->col, parser_error->content);
        }
        llex_free(tokens);
        last_free(ast);
        lval_mut_err(r, LERR_EVAL);
        return false;
    }
    /* Evaluate AST. */
    bool ret = false;
    struct last *eval_error = NULL; // An eval error occurs at a specific location of the AST.
    if (!(ret = leval_ast(ast, r, &eval_error)) && eval_error) {
        if (prompt_len >= 0) {
            print_error_marker(stderr, prompt_len, eval_error->col);
            size_t len = lval_printlen(r);
            char* err = calloc(1, len);
            lval_as_str(r, err, len);
            fprintf(stderr, "<stdin>:%d:%d: evaluation error: %s.\n",
                    eval_error->line, eval_error->col, err);
            free(err);
        }
    }
    /* Cleanup */
    llex_free(tokens);
    last_free(ast);
    return ret;
}

void lisp_eval_from_string(const char* restrict input, int prompt_len) {
    struct lval* r = lval_alloc();
    if (lisp_eval(input, r, prompt_len)) {
        lval_println(r);
    } // Error already printed.
    lval_free(r);
}
