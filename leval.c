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

static bool leval_exec(char* op, const struct lval* x, const struct lval* y, struct lval* r) {
    if (lval_type(x) == LVAL_ERR) {
        lval_copy(r, x);
        return false;
    }
    if (lval_type(y) == LVAL_ERR) {
        lval_copy(r, y);
        return false;
    }

    if (strcmp(op, "+") == 0) return lsym_exec(lbuiltin_op_add, x, y, r);
    if (strcmp(op, "-") == 0) return lsym_exec(lbuiltin_op_sub, x, y, r);
    if (strcmp(op, "*") == 0) return lsym_exec(lbuiltin_op_mul, x, y, r);
    if (strcmp(op, "/") == 0) return lsym_exec(lbuiltin_op_div, x, y, r);
    if (strcmp(op, "%") == 0) return lsym_exec(lbuiltin_op_mod, x, y, r);
    if (strcmp(op, "^") == 0) return lsym_exec(lbuiltin_op_pow, x, y, r);
    if (strcmp(op, "!") == 0) {
        if (lval_type(y) != LVAL_NIL) {
            return lval_mut_err(r, LERR_TOO_MANY_ARGS);
            return false;
        }
        return lsym_exec(lbuiltin_op_fac, x, &lnil, r);
    }

    lval_mut_err(r, LERR_BAD_SYMBOL);
    return false;
}

static bool leval_num(struct last* ast, struct lval* r) {
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

static bool leval_dbl(struct last* ast, struct lval* r) {
    errno = 0;
    double d = strtod(ast->content, NULL);
    if (errno == ERANGE) {
        lval_mut_err(r, LERR_BAD_OPERAND);
        return false;
    }
    lval_mut_dbl(r, d);
    return true;
}

static bool leval_ast(struct last* ast, struct lval* r);

static bool leval_expr(struct last* ast, struct lval* r) {
    char* op = ast->children[0]->content;
    struct lval *x = NULL, *y = NULL;
    switch (ast->childrenc - 1) {
    case 0: // no arguments.
        leval_exec(op, &lnil, &lnil, r);
        break;
    case 1: // 1 argument.
        x = lval_alloc();
        leval_ast(ast->children[1], x);
        leval_exec(op, x, &lnil, r);
        lval_free(x);
        break;
    default: // >= 2
        x = lval_alloc();
        leval_ast(ast->children[1], x);
        lval_copy(r, x);
        for (size_t i = 2; i < ast->childrenc; i++) {
            y = lval_alloc();
            leval_ast(ast->children[i], y);
            leval_exec(op, r, y, r);
            lval_free(y);
        }
        lval_free(x);
        break;
    }
    return true;
}

static bool leval_ast(struct last* ast, struct lval* r) {
    if (!ast) {
        lval_mut_err(r, LERR_EVAL);
        return false;
    }
    switch (ast->tag) {
    case LTAG_PROG:
        return leval_ast(ast->children[0], r);
    case LTAG_NUM:
        return leval_num(ast, r);
    case LTAG_DBL:
        return leval_dbl(ast, r);
    case LTAG_SEXPR:
        return leval_expr(ast->children[0], r);
    case LTAG_EXPR:
        return leval_expr(ast, r);
    default: break;
    }
    lval_mut_err(r, LERR_EVAL);
    return false;
}

bool lisp_eval(const char* restrict input, struct lval* r) {
    /* Lex input. */
    struct ltok *tokens = NULL, *lexer_error = NULL;
    tokens = lisp_lex(input, &lexer_error);
    if (lexer_error != NULL) {
        fprintf(stderr, "Lexing error at line %d column %d: %s.\n",
                lexer_error->line, lexer_error->col, lexer_error->content);
        llex_free(tokens);
        return false;
    }
    /* Parse input. */
    struct last *ast, *parser_error = NULL;
    ast = lisp_parse(tokens, &parser_error);
    if (parser_error != NULL) {
        fprintf(stderr, "Parsing error at line %d column %d: %s.\n",
                parser_error->line, parser_error->col, parser_error->content);
        llex_free(tokens);
        last_free(ast);
        return false;
    }
    bool ret = false;
    ret = leval_ast(ast, r); // Skip program node.
    /* Cleanup */
    llex_free(tokens);
    last_free(ast);
    return ret;
}

void lisp_eval_from_string(const char* restrict input) {
    struct lval* r = lval_alloc();
    lisp_eval(input, r);
    lval_println(r);
    lval_free(r);
}
