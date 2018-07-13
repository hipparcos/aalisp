#include "polish.h"

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "vendor/mpc/mpc.h"
#include "lval.h"

/* Private prototypes */
static struct lval polish_eval_unary_op(struct lval x, char* op);
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
                  operator : '+' | '-' | '*' | '/' | '%' | '!' ;                \
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
    struct lval x = polish_eval_expr(ast->children[2]);

    if (strstr(ast->children[3]->tag, "expr")) {
        for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
            x = polish_eval_op(x, op, polish_eval_expr(ast->children[i]));
        }
    } else {
        x = polish_eval_unary_op(x, op);
    }

    return x;
}

/* Operators evaluation */
#define EITHER_IS_DBL(x,y) (x.type == LVAL_DBL || y.type == LVAL_DBL)
#define EITHER_IS_BIGNUM(x,y) (x.type == LVAL_BIGNUM || y.type == LVAL_BIGNUM)

static struct lval polish_op_add(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y) && EITHER_IS_BIGNUM(x,y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) + lval_as_dbl(y));
    } else if (EITHER_IS_BIGNUM(x,y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t res;
        mpz_init(res);
        mpz_add(res, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(res);
        mpz_clear(res);
        return ret;
    } else {
        long a = x.data.num;
        long b = y.data.num;
        if (a >= LONG_MAX - b) {
            mpz_t* bnx = lval_as_bignum(x);
            mpz_t* bny = lval_as_bignum(y);
            struct lval res = polish_op_add(lval_bignum(*bnx), lval_bignum(*bny));
            mpz_clear(*bnx);
            mpz_clear(*bny);
            return res;
        }
        return lval_num(x.data.num + y.data.num);
    }
}

static struct lval polish_op_sub(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y) && EITHER_IS_BIGNUM(x,y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) - lval_as_dbl(y));
    } else if (EITHER_IS_BIGNUM(x,y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t res;
        mpz_init(res);
        mpz_sub(res, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(res);
        mpz_clear(res);
        return ret;
    } else {
        long a = x.data.num;
        long b = y.data.num;
        if (a <= LONG_MIN + b) {
            mpz_t* bnx = lval_as_bignum(x);
            mpz_t* bny = lval_as_bignum(y);
            struct lval res = polish_op_sub(lval_bignum(*bnx), lval_bignum(*bny));
            mpz_clear(*bnx);
            mpz_clear(*bny);
            return res;
        }
        return lval_num(x.data.num - y.data.num);
    }
}

static struct lval polish_op_mul(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y) && EITHER_IS_BIGNUM(x,y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) * lval_as_dbl(y));
    } else if (EITHER_IS_BIGNUM(x,y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t res;
        mpz_init(res);
        mpz_mul(res, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(res);
        mpz_clear(res);
        return ret;
    } else {
        long a = x.data.num;
        long b = y.data.num;
        if ((b > 0 && ((a > 0 && a > LONG_MAX / b) || (a < 0 && a < LONG_MIN / b))) ||
                (b == -1 && a == -LONG_MAX) ||
                (b < -1 && ((a < 0 && a < LONG_MAX / b) || (a > 0 && a > LONG_MIN / b)))) {
            mpz_t* bnx = lval_as_bignum(x);
            mpz_t* bny = lval_as_bignum(y);
            struct lval res = polish_op_mul(lval_bignum(*bnx), lval_bignum(*bny));
            mpz_clear(*bnx);
            mpz_clear(*bny);
            return res;
        }
        return lval_num(x.data.num * y.data.num);
    }
}

static struct lval polish_op_div(struct lval x, struct lval y) {
    if (lval_is_zero(y)) {
        return lval_err(LERR_DIV_ZERO);
    }
    if (EITHER_IS_DBL(x,y) && EITHER_IS_BIGNUM(x,y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) / lval_as_dbl(y));
    } else if (EITHER_IS_BIGNUM(x,y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t res;
        mpz_init(res);
        mpz_fdiv_q(res, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(res);
        mpz_clear(res);
        return ret;
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
    if (EITHER_IS_DBL(x,y) && EITHER_IS_BIGNUM(x,y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (EITHER_IS_BIGNUM(x,y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t res;
        mpz_init(res);
        mpz_mod(res, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(res);
        mpz_clear(res);
        return ret;
    }

    return lval_num(x.data.num % y.data.num);
}

static struct lval polish_op_fact(struct lval x) {
    if (x.type == LVAL_DBL) {
        return lval_err(LERR_BAD_NUM);
    }
    if (x.type == LVAL_NUM && x.data.num < 0) {
        return lval_err(LERR_BAD_NUM);
    }
    if (x.type == LVAL_BIGNUM && mpz_sgn(x.data.bignum) < 0) {
        return lval_err(LERR_BAD_NUM);
    }
    if (x.type == LVAL_BIGNUM && mpz_cmp_si(x.data.bignum, ULONG_MAX) > 0) {
        return lval_err(LERR_BAD_NUM);
    }

    if (lval_is_zero(x)) {
        return lval_num(1);
    }

    /* Up to long. */
    if (x.type == LVAL_NUM && x.data.num < 21) {
        int n = x.data.num;
        long fact = n;
        while (n > 2) {
            fact *= --n;
        }
        return lval_num(fact);
    }

    /* Always use bignum from here. */
    mpz_t* arg = lval_as_bignum(x);
    unsigned long n = mpz_get_ui(*arg);
    mpz_t fact;
    mpz_init(fact);
    mpz_fac_ui(fact, n);
    struct lval ret = lval_bignum(fact);
    mpz_clear(fact);
    mpz_clear(*arg);
    return ret;
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

static struct lval polish_eval_unary_op(struct lval x, char* op) {
    if (x.type == LVAL_ERR) return x;

    if (strcmp(op, "!") == 0) return polish_op_fact(x);

    return lval_err(LERR_BAD_NUM);
}
