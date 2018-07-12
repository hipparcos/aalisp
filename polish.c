#include "polish.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "vendor/mpc/mpc.h"
#include "lval.h"

/* Private prototypes */
static struct lval polish_eval_op(struct lval x, char* op, struct lval y);
static struct lval polish_eval_expr(mpc_ast_t* ast);

/* Parser */
mpc_parser_t* Polish    = NULL;

void polish_eval(const char* restrict input) {
    if (!Polish)
        polish_setup();

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Polish, &r)) {
        struct lval result = polish_eval_expr(r.output);
        lval_println(result);
        mpc_ast_delete(r.output);
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }
}

/* Init & Cleanup */
mpc_parser_t* Number    = NULL;
mpc_parser_t* Double    = NULL;
mpc_parser_t* Operator  = NULL;
mpc_parser_t* Expr      = NULL;

void polish_setup() {
    if (!Polish) {
        Number    = mpc_new("number");
        Double    = mpc_new("double");
        Operator  = mpc_new("operator");
        Expr      = mpc_new("expr");
        Polish    = mpc_new("polish");

        /* double must be matched before number. */
        mpca_lang(MPCA_LANG_DEFAULT,
                  "                                                             \
                  double   : /-?[0-9]*\\.[0-9]+/ ;                              \
                  number   : /-?[0-9]+/ ;                                       \
                  operator : '+' | '-' | '*' | '/' | '%' ;                      \
                  expr     : <double> | <number> | '(' <operator> <expr>+ ')' ; \
                  polish   : /^/ <operator> <expr>+ /$/ ;                       \
                  ",
                  Number, Double, Operator, Expr, Polish);
    }
}

void polish_cleanup() {
    mpc_cleanup(5, Number, Double, Operator, Expr, Polish);
    Number = NULL;
    Double = NULL;
    Operator = NULL;
    Expr = NULL;
    Polish = NULL;
}

/* Expression evaluation */
static struct lval polish_eval_expr(mpc_ast_t* ast) {
    if (strstr(ast->tag, "number")) {
        errno = 0;
        long x = strtol(ast->contents, NULL, 10);
        if (errno == ERANGE) {
            return lval_err(LERR_BAD_NUM);
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
    struct lval x = polish_eval_expr(ast->children[2]);

    for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
        x = polish_eval_op(x, op, polish_eval_expr(ast->children[i]));
    }

    return x;
}

/* Operators evaluation */
#define EITHER_IS_DBL(x,y) x.type == LVAL_DBL || y.type == LVAL_DBL

static struct lval polish_op_add(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) + lval_as_dbl(y));
    } else {
        return lval_num(x.data.num + y.data.num);
    }
}

static struct lval polish_op_sub(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) - lval_as_dbl(y));
    } else {
        return lval_num(x.data.num - y.data.num);
    }
}

static struct lval polish_op_mul(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) * lval_as_dbl(y));
    } else {
        return lval_num(x.data.num * y.data.num);
    }
}

static struct lval polish_op_div(struct lval x, struct lval y) {
    if (lval_is_zero(y)) {
        return lval_err(LERR_DIV_ZERO);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) / lval_as_dbl(y));
    } else {
        return lval_num(x.data.num / y.data.num);
    }
}

static struct lval polish_op_mod(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x, y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (lval_is_zero(y)) {
        return lval_err(LERR_DIV_ZERO);
    }

    return lval_num(x.data.num % y.data.num);
}

static struct lval polish_eval_op(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return polish_op_add(x, y);
    if (strcmp(op, "-") == 0) return polish_op_sub(x, y);
    if (strcmp(op, "*") == 0) return polish_op_mul(x, y);
    if (strcmp(op, "/") == 0) return polish_op_div(x, y);
    if (strcmp(op, "%") == 0) return polish_op_mod(x, y);

    return lval_err(LERR_BAD_NUM);
}
