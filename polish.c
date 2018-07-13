#include "polish.h"

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "vendor/mpc/mpc.h"
#include "lval.h"
#include "operator.h"

/* Private prototypes */
void polish_declare_operators();
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
                  operator : '+' | '-' | '*' | '/' | '%' | '!' | '^' ;          \
                  expr     : <double> | <number> | '(' <operator> <expr>+ ')' ; \
                  polish   : /^/ <operator> <expr>+ /$/ ;                       \
                  ",
                  Number, Double, Operator, Expr, Polish);

        polish_declare_operators();
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

    /* Multiple operand. */
    if (strstr(ast->children[3]->tag, "expr")) {
        for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
            x = polish_eval_op(x, op, polish_eval_expr(ast->children[i]));
        }
    /* One operand. */
    } else {
        x = polish_eval_op(x, op, lval_nil());
    }

    return x;
}

/* Conditions: eval a condition for the given operands. */
typedef bool (*condition_func)(struct lval, struct lval);

/* Guard: a condition associated with an error. */
struct guard {
    condition_func condition;
    enum lerr error;
};

/* Operators: generic */
struct op_descriptor {
    const char*         symbol;
    bool                unary;
    const struct guard* guards;
    int                 guardc;
    struct lval         neutral;
    long   (*op_num)(const long, const long);
    bool   (*cnd_overflow)(const long, const long);
    void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y);
    double (*op_dbl)(const double, const double);
};

static struct lval polish_op(
    const struct op_descriptor descriptor,
    const struct lval x, struct lval y
) {

    if (!descriptor.unary && y.type == LVAL_NIL) {
        y = descriptor.neutral;
    }

    /* Guards */
    for (int i = 0; i < descriptor.guardc; i++) {
        if ((descriptor.guards[i].condition)(x, y)) {
            return lval_err(descriptor.guards[i].error);
        }
    }

    /* Eval: double */
    if (cnd_either_is_dbl(x, y)) {
        double a = lval_as_dbl(x);
        double b = lval_as_dbl(y);
        return lval_dbl(descriptor.op_dbl(a, b));
    }
    /* Eval: bignum */
    if (cnd_either_is_bignum(x, y)) {
        mpz_t* a = lval_as_bignum(x);
        mpz_t* b = lval_as_bignum(y);
        mpz_t r;
        mpz_init(r);
        descriptor.op_bignum(r, *a, *b);
        mpz_clear(*a);
        mpz_clear(*b);
        struct lval ret = lval_bignum(r);
        mpz_clear(r);
        return ret;
    }
    /* Eval: num */
    if (cnd_are_num(x, y)) {
        long a = x.data.num;
        long b = y.data.num;
        if (descriptor.cnd_overflow && descriptor.cnd_overflow(a, b)) {
            mpz_t* bna = lval_as_bignum(x);
            mpz_t* bnb = lval_as_bignum(y);
            struct lval res = polish_op(descriptor, lval_bignum(*bna), lval_bignum(*bnb));
            mpz_clear(*bna);
            mpz_clear(*bnb);
            return res;
        }
        return lval_num(descriptor.op_num(a, b));
    }

    return lval_err(LERR_EVAL);
}

/* Guards: instanciation. */
const struct guard guard_x_is_negative = {
    .condition= cnd_x_is_neg,
    .error= LERR_BAD_NUM
};
const struct guard guard_y_is_negative = {
    .condition= cnd_y_is_neg,
    .error= LERR_BAD_NUM
};
const struct guard guard_x_too_big = {
    .condition= cnd_x_too_big_for_ul,
    .error= LERR_BAD_NUM
};
const struct guard guard_y_too_big = {
    .condition= cnd_y_too_big_for_ul,
    .error= LERR_BAD_NUM
};
const struct guard guard_div_by_zero = {
    .condition= cnd_y_is_zero,
    .error= LERR_DIV_ZERO
};
const struct guard guard_either_is_double = {
    .condition= cnd_either_is_dbl,
    .error= LERR_BAD_NUM
};
const struct guard guard_dbl_and_bignum = {
    .condition= cnd_dbl_and_bignum,
    .error= LERR_BAD_NUM
};

/* Operator: declaration */
struct op_descriptor op_add;
struct op_descriptor op_sub;
struct op_descriptor op_mul;
struct guard op_div_guards[2];
struct op_descriptor op_div;
struct guard op_mod_guards[3];
struct op_descriptor op_mod;
struct guard op_fac_guards[3];
struct op_descriptor op_fac;
struct guard op_pow_guards[3];
struct op_descriptor op_pow;

void polish_declare_operators() {
    op_add.symbol = "+";
    op_add.guards = &guard_dbl_and_bignum;
    op_add.guardc = 1;
    op_add.neutral = lval_num(0);
    op_add.op_num       = op_num_add;
    op_add.cnd_overflow = cnd_num_add_overflow;
    op_add.op_bignum    = mpz_add;
    op_add.op_dbl       = op_dbl_add;

    op_sub.symbol = "-";
    op_sub.guards = &guard_dbl_and_bignum;
    op_sub.guardc = 1;
    op_sub.neutral = lval_num(0);
    op_sub.op_num = op_num_sub;
    op_sub.cnd_overflow = cnd_num_sub_overflow;
    op_sub.op_bignum = mpz_sub;
    op_sub.op_dbl = op_dbl_sub;

    op_mul.symbol = "*";
    op_mul.guards = &guard_dbl_and_bignum;
    op_mul.guardc = 1;
    op_mul.neutral = lval_num(1);
    op_mul.op_num = op_num_mul;
    op_mul.cnd_overflow = cnd_num_mul_overflow;
    op_mul.op_bignum = mpz_mul;
    op_mul.op_dbl = op_dbl_mul;

    op_div.symbol = "/";
    op_div.guards = op_div_guards;
    op_div.guardc = 2;
    op_div.neutral = lval_num(1);
    op_div.op_num = op_num_div;
    op_div.cnd_overflow = NULL;
    op_div.op_bignum = mpz_fdiv_q;
    op_div.op_dbl = op_dbl_div;
    op_div_guards[0] = guard_div_by_zero;
    op_div_guards[1] = guard_dbl_and_bignum;

    op_mod.symbol = "%";
    op_mod.guards = op_mod_guards;
    op_mod.guardc = 3;
    op_mod.neutral = lval_num(1);
    op_mod.op_num = op_num_mod;
    op_mod.cnd_overflow = NULL;
    op_mod.op_bignum = mpz_mod;
    op_mod.op_dbl = op_dbl_nop;
    op_mod_guards[0] = guard_either_is_double;
    op_mod_guards[1] = guard_div_by_zero;
    op_mod_guards[2] = guard_dbl_and_bignum;

    op_fac.symbol = "!";
    op_fac.unary = true;
    op_fac.guards = op_fac_guards;
    op_fac.guardc = 3;
    op_fac.op_num = op_num_fact;
    op_fac.cnd_overflow = cnd_num_fact_overflow;
    op_fac.op_bignum = op_bignum_fac;
    op_fac.op_dbl = op_dbl_nop;
    op_fac_guards[0] = guard_either_is_double;
    op_fac_guards[1] = guard_x_is_negative;
    op_fac_guards[2] = guard_x_too_big;

    op_pow.symbol = "^";
    op_pow.guards = op_pow_guards;
    op_pow.guardc = 3;
    op_mod.neutral = lval_num(1);
    op_pow.op_num = op_num_pow;
    op_pow.cnd_overflow = cnd_num_pow_overflow;
    op_pow.op_bignum = op_bignum_pow;
    op_pow.op_dbl = op_dbl_pow;
    op_pow_guards[0] = guard_dbl_and_bignum;
    op_pow_guards[1] = guard_y_is_negative;
    op_pow_guards[2] = guard_y_too_big;
}

static struct lval polish_eval_op(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return polish_op(op_add, x, y);
    if (strcmp(op, "-") == 0) return polish_op(op_sub, x, y);
    if (strcmp(op, "*") == 0) return polish_op(op_mul, x, y);
    if (strcmp(op, "/") == 0) return polish_op(op_div, x, y);
    if (strcmp(op, "%") == 0) return polish_op(op_mod, x, y);
    if (strcmp(op, "^") == 0) return polish_op(op_pow, x, y);
    if (strcmp(op, "!") == 0) {
        if (y.type != LVAL_NIL) {
            return lval_err(LERR_TOO_MANY_ARGS);
        }
        return polish_op(op_fac, x, lval_nil());
    }

    return lval_err(LERR_BAD_NUM);
}
