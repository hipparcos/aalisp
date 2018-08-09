#include "lmut.h"

#include <errno.h>

static struct lval* lmut_num(const struct last* ast, struct last** error) {
    (void)error;
    errno = 0;
    long n = strtol(ast->content, NULL, 10);
    struct lval* v = lval_alloc();
    v->ast = ast;
    if (errno == ERANGE) {
        /* Switch to bignum. */
        mpz_t bignum;
        mpz_init_set_str(bignum, ast->content, 10);
        lval_mut_bignum(v, bignum);
        mpz_clear(bignum);
        return v;
    }
    lval_mut_num(v, n);
    return v;
}

static struct lval* lmut_dbl(const struct last* ast, struct last** error) {
    errno = 0;
    double d = strtod(ast->content, NULL);
    struct lval* v = lval_alloc();
    v->ast = ast;
    if (errno == ERANGE) {
        lval_mut_err(v, LERR_BAD_OPERAND);
        *error = (struct last*)ast;
        return v;
    }
    lval_mut_dbl(v, d);
    return v;
}

static struct lval* lmut_sym(const struct last* ast, struct last** error) {
    *error = NULL;
    struct lval* v = lval_alloc();
    lval_mut_sym(v, ast->content);
    v->ast = ast;
    return v;
}

static struct lval* lmut_str(const struct last* ast, struct last** error) {
    *error = NULL;
    struct lval* v = lval_alloc();
    lval_mut_str(v, ast->content);
    v->ast = ast;
    return v;
}

static struct lval* lmut_sexpr(const struct last* ast, struct last** error) {
    struct lval* v = lval_alloc();
    lval_mut_sexpr(v);
    v->ast = ast;
    /* Dereference the inner expression. */
    ast = ast->children[0];
    for (size_t c = 0; c < ast->childrenc; c++) {
        struct lval* o = NULL;
        switch (ast->children[c]->tag) {
        case LTAG_NUM: o = lmut_num(ast->children[c], error); break;
        case LTAG_DBL: o = lmut_dbl(ast->children[c], error); break;
        case LTAG_SYM: o = lmut_sym(ast->children[c], error); break;
        case LTAG_STR: o = lmut_str(ast->children[c], error); break;
        case LTAG_SEXPR: o = lmut_sexpr(ast->children[c], error); break;
        default:
            o = lval_alloc();
            lval_mut_err(o, LERR_AST);
            o->ast = ast;
            *error = (struct last*)ast;
            break;
        }
        lval_push(v, o);
        /* Stop on error. */
        if (*error != NULL) {
            break;
        }
    }
    return v;
}

struct lval* lisp_mut(const struct last* ast, struct last** error) {
    if (!ast) {
        *error = NULL;
        return NULL;
    }
    struct lval* p = lval_alloc();
    if (ast->tag != LTAG_PROG) {
        lval_mut_err(p, LERR_AST);
        *error = NULL;
        return p;
    }
    /* A program is a list of SEXPR. */
    lval_mut_sexpr(p);
    p->ast = ast;
    for (size_t c = 0; c < ast->childrenc; c++) {
        struct lval* s = NULL;
        if (ast->children[c]->tag == LTAG_SEXPR) {
            s = lmut_sexpr(ast->children[c], error);
        } else {
            s = lval_alloc();
            lval_mut_err(s, LERR_AST);
            s->ast = ast->children[c];
            *error = (struct last*)ast->children[c];
        }
        lval_push(p, s);
        if (*error != NULL) {
            break;
        }
    }
    return p;
}
