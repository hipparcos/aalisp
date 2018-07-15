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

static struct lval lisp_eval_symbol(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return lsym_exec(lbuiltin_op_add, x, y);
    if (strcmp(op, "-") == 0) return lsym_exec(lbuiltin_op_sub, x, y);
    if (strcmp(op, "*") == 0) return lsym_exec(lbuiltin_op_mul, x, y);
    if (strcmp(op, "/") == 0) return lsym_exec(lbuiltin_op_div, x, y);
    if (strcmp(op, "%") == 0) return lsym_exec(lbuiltin_op_mod, x, y);
    if (strcmp(op, "^") == 0) return lsym_exec(lbuiltin_op_pow, x, y);
    if (strcmp(op, "!") == 0) {
        if (y.type != LVAL_NIL) {
            return lval_err(LERR_TOO_MANY_ARGS);
        }
        return lsym_exec(lbuiltin_op_fac, x, lval_nil());
    }

    return lval_err(LERR_BAD_NUM);
}

static struct lval lisp_eval_expr(mpc_ast_t* ast) {
    if (strstr(ast->tag, "number")) {
        errno = 0;
        long x = strtol(ast->contents, NULL, 10);
        if (errno == ERANGE) {
            /* Switch to bignum. */
            mpz_t bignum;
            mpz_init_set_str(bignum, ast->contents, 10);
            struct lval ret = lval_bignum(bignum);
            mpz_clear(bignum);
            return ret;
        }
        return lval_num(x);
    }
    if (strstr(ast->tag, "double")) {
        errno = 0;
        double x = strtod(ast->contents, NULL);
        if (errno == ERANGE) {
            return lval_err(LERR_BAD_NUM);
        }
        return lval_dbl(x);
    }

    /* Operator always is 2nd child. */
    char* op = ast->children[1]->contents;

    /* Eval the remaining children. */
    struct lval x = lisp_eval_expr(ast->children[2]);

    struct lval res = lval_nil();
    /* Multiple operand. */
    if (strstr(ast->children[3]->tag, "expr")) {
        for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
            struct lval y = lisp_eval_expr(ast->children[i]);
            res = lisp_eval_symbol((res.type == LVAL_NIL) ? x : res, op, y);
            lval_clear(&y);
        }
    /* One operand. */
    } else {
        res = lisp_eval_symbol(x, op, lval_nil());
    }
    lval_clear(&x);

    return res;
}

void lisp_eval(const char* restrict input) {
    if (!Lisp)
        lisp_setup();

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lisp, &r)) {
        struct lval result = lisp_eval_expr(r.output);
        lval_println(result);
        mpc_ast_delete(r.output);
        lval_clear(&result);
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
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
