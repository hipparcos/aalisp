#include "lmut.h"

#include <errno.h>

#include "lparser.h"
#include "lerr.h"
#include "lval.h"

static struct lval* lmut_num(const struct last* ast, struct lerr** error) {
    (void)error;
    errno = 0;
    long n = strtol(ast->content, NULL, 10);
    struct lval* v = lval_alloc();
    if (errno == ERANGE) {
        /* Switch to bignum. */
        mpz_t bignum;
        mpz_init_set_str(bignum, ast->content, 10);
        lval_mut_bignum(v, bignum);
        v->ast = ast;
        mpz_clear(bignum);
        return v;
    }
    lval_mut_num(v, n);
    v->ast = ast;
    return v;
}

static struct lval* lmut_dbl(const struct last* ast, struct lerr** error) {
    errno = 0;
    double d = strtod(ast->content, NULL);
    struct lval* v = lval_alloc();
    if (errno == ERANGE) {
        *error = lerr_throw(LERR_BAD_OPERAND, "double number out of range");
        lerr_set_location(*error, ast->line, ast->col);
        lval_mut_err_ptr(v, *error);
        v->ast = ast;
        return v;
    }
    lval_mut_dbl(v, d);
    v->ast = ast;
    return v;
}

static struct lval* lmut_sym(const struct last* ast, struct lerr** error) {
    *error = NULL;
    struct lval* v = lval_alloc();
    lval_mut_sym(v, ast->content);
    v->ast = ast;
    return v;
}

static struct lval* lmut_str(const struct last* ast, struct lerr** error) {
    *error = NULL;
    struct lval* v = lval_alloc();
    lval_mut_str(v, ast->content);
    v->ast = ast;
    return v;
}

static struct lval* lmut_sexpr(const struct last* ast, struct lerr** error);
static struct lval* lmut_qexpr(const struct last* ast, struct lerr** error);

void lmut_fill_list(struct lval* list, const struct last* ast, struct lerr** error) {
    for (size_t c = 0; c < ast->childrenc; c++) {
        struct lval* o = NULL;
        switch (ast->children[c]->tag) {
        case LTAG_NUM: o = lmut_num(ast->children[c], error); break;
        case LTAG_DBL: o = lmut_dbl(ast->children[c], error); break;
        case LTAG_SYM: o = lmut_sym(ast->children[c], error); break;
        case LTAG_STR: o = lmut_str(ast->children[c], error); break;
        case LTAG_SEXPR: o = lmut_sexpr(ast->children[c], error); break;
        case LTAG_QEXPR: o = lmut_qexpr(ast->children[c], error); break;
        default:
            o = lval_alloc();
            *error = lerr_throw(LERR_AST, "can't read AST");
            lerr_set_location(*error, ast->line, ast->col);
            lval_mut_err_ptr(o, *error);
            o->ast = ast;
            break;
        }
        lval_push(list, o);
        lval_free(o);
        /* Stop on error. */
        if (*error != NULL) {
            break;
        }
    }
}

static struct lval* lmut_qexpr(const struct last* ast, struct lerr** error) {
    struct lval* v = lval_alloc();
    lval_mut_qexpr(v);
    v->ast = ast;
    /* Add children to the qexpr. */
    lmut_fill_list(v, ast, error);
    return v;
}

static struct lval* lmut_sexpr(const struct last* ast, struct lerr** error) {
    struct lval* v = lval_alloc();
    lval_mut_sexpr(v);
    v->ast = ast;
    /* Dereference the inner expression. */
    ast = ast->children[0];
    /* Add children to the sexpr. */
    lmut_fill_list(v, ast, error);
    return v;
}

struct lval* lisp_mut(const struct last* ast, struct lerr** error) {
    if (!ast) {
        *error = NULL;
        return NULL;
    }
    struct lval* p = lval_alloc();
    if (ast->tag != LTAG_PROG) {
        struct lerr* err = lerr_throw(LERR_AST, "can't read AST");
        lval_mut_err_ptr(p, err);
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
            *error = lerr_throw(LERR_AST, "can't read AST");
            lerr_set_location(*error, ast->line, ast->col);
            lval_mut_err_ptr(s, *error);
            s->ast = ast->children[c];
        }
        lval_push(p, s);
        lval_free(s);
        if (*error != NULL) {
            break;
        }
    }
    return p;
}
