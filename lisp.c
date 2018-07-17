#include "lisp.h"

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "vendor/mpc/mpc.h"

#include "lbuiltin.h"
#include "lval.h"
#include "lsym.h"

/* Parser */
static mpc_parser_t* Lisp   = NULL;
static mpc_parser_t* Number = NULL;
static mpc_parser_t* Double = NULL;
static mpc_parser_t* Symbol = NULL;
static mpc_parser_t* Expr   = NULL;
static mpc_parser_t* Sexpr  = NULL;

void lisp_setup(void) {
    if (!Lisp) {
        Double = mpc_new("double");
        Number = mpc_new("number");
        Symbol = mpc_new("symbol");
        Sexpr  = mpc_new("sexpr");
        Expr   = mpc_new("expr");
        Lisp   = mpc_new("lisp");

        /* double must be matched before number. */
        mpca_lang(MPCA_LANG_DEFAULT,
                  "                                                       \
                  double   : /-?[0-9]*\\.[0-9]+/ ;                        \
                  number   : /-?[0-9]+/ ;                                 \
                  symbol   : '+' | '-' | '*' | '/' | '%' | '!' | '^' ;    \
                  sexpr    : '(' <expr>* ')' ;                            \
                  expr     : <double> | <number> | <symbol> | <sexpr> ;   \
                  lisp     : /^/ <expr>* /$/ ;                            \
                  ",
                  Double, Number, Symbol, Sexpr, Expr, Lisp);
    }
}

bool lisp_eval_symbol(char* op, const struct lval* x, const struct lval* y, struct lval* r) {
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

static bool lisp_eval_expr(mpc_ast_t* ast, struct lval* r) {
    if (strstr(ast->tag, "number")) {
        errno = 0;
        long x = strtol(ast->contents, NULL, 10);
        if (errno == ERANGE) {
            /* Switch to bignum. */
            mpz_t bignum;
            mpz_init_set_str(bignum, ast->contents, 10);
            lval_mut_bignum(r, bignum);
            mpz_clear(bignum);
            return true;
        }
        lval_mut_num(r, x);
        return true;
    }
    if (strstr(ast->tag, "double")) {
        errno = 0;
        double x = strtod(ast->contents, NULL);
        if (errno == ERANGE) {
            lval_mut_err(r, LERR_BAD_OPERAND);
            return false;
        }
        lval_mut_dbl(r, x);
        return true;
    }

    /* Operator always is 2nd child. */
    char* op = ast->children[1]->contents;

    /* Eval the remaining children. */
    struct lval* x = lval_alloc();
    lisp_eval_expr(ast->children[2], x);

    lval_copy(r, x);
    /* Multiple operand. */
    if (strstr(ast->children[3]->tag, "expr")) {
        for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
            struct lval* y = lval_alloc();
            lisp_eval_expr(ast->children[i], y);
            lisp_eval_symbol(op, r, y, r);
            lval_free(y);
        }
    /* One operand. */
    } else {
        lisp_eval_symbol(op, x, &lnil, r);
    }
    lval_free(x);
    return true;
}

void lisp_eval(const char* restrict input) {
    if (!Lisp)
        lisp_setup();

    mpc_result_t ast;
    if (mpc_parse("<stdin>", input, Lisp, &ast)) {
        struct lval* r = lval_alloc();
        lisp_eval_expr(ast.output, r);
        lval_println(r);
        lval_free(r);
        mpc_ast_delete(ast.output);
    } else {
        mpc_err_print(ast.error);
        mpc_err_delete(ast.error);
    }
}

void lisp_teardown(void) {
    mpc_cleanup(6, Double, Number, Symbol, Sexpr, Expr, Lisp);
    Double = NULL;
    Number = NULL;
    Symbol = NULL;
    Sexpr  = NULL;
    Expr   = NULL;
    Lisp   = NULL;
}
