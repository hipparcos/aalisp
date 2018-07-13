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
static mpc_parser_t* Polish    = NULL;

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
static mpc_parser_t* Number    = NULL;
static mpc_parser_t* Double    = NULL;
static mpc_parser_t* Operator  = NULL;
static mpc_parser_t* Expr      = NULL;

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
    const char*          symbol;
    bool                 unary;
    const struct guard** guards;
    int                  guardc;
    const struct lval*   neutral;
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
        y = *descriptor.neutral;
    }

    /* Guards */
    for (int i = 0; i < descriptor.guardc; i++) {
        if ((descriptor.guards[i]->condition)(x, y)) {
            return lval_err(descriptor.guards[i]->error);
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
static const struct guard guard_x_is_negative = {
    .condition= cnd_x_is_neg,
    .error= LERR_BAD_NUM
};
static const struct guard guard_y_is_negative = {
    .condition= cnd_y_is_neg,
    .error= LERR_BAD_NUM
};
static const struct guard guard_x_too_big = {
    .condition= cnd_x_too_big_for_ul,
    .error= LERR_BAD_NUM
};
static const struct guard guard_y_too_big = {
    .condition= cnd_y_too_big_for_ul,
    .error= LERR_BAD_NUM
};
static const struct guard guard_div_by_zero = {
    .condition= cnd_y_is_zero,
    .error= LERR_DIV_ZERO
};
static const struct guard guard_either_is_double = {
    .condition= cnd_either_is_dbl,
    .error= LERR_BAD_NUM
};
static const struct guard guard_dbl_and_bignum = {
    .condition= cnd_dbl_and_bignum,
    .error= LERR_BAD_NUM
};

/* Operator: declaration */
#define LENGTH(array) sizeof(array)/sizeof(array[0])

static const struct guard* guards_op_add[] = {
    &guard_dbl_and_bignum
};
static const struct op_descriptor op_add = {
    .symbol       = "+",
    .guards       = guards_op_add,
    .guardc       = LENGTH(guards_op_add),
    .neutral      = &lzero,
    .op_num       = op_num_add,
    .cnd_overflow = cnd_num_add_overflow,
    .op_bignum    = mpz_add,
    .op_dbl       = op_dbl_add
};

static const struct guard* guards_op_sub[] = {
    &guard_dbl_and_bignum
};
static const struct op_descriptor op_sub = {
    .symbol       = "-",
    .guards       = guards_op_sub,
    .guardc       = LENGTH(guards_op_sub),
    .neutral      = &lzero,
    .op_num       = op_num_sub,
    .cnd_overflow = cnd_num_sub_overflow,
    .op_bignum    = mpz_sub,
    .op_dbl       = op_dbl_sub
};

static const struct guard* guards_op_mul[] = {
    &guard_dbl_and_bignum
};
static const struct op_descriptor op_mul = {
    .symbol       = "*",
    .guards       = guards_op_mul,
    .guardc       = LENGTH(guards_op_mul),
    .neutral      = &lone,
    .op_num       = op_num_mul,
    .cnd_overflow = cnd_num_mul_overflow,
    .op_bignum    = mpz_mul,
    .op_dbl       = op_dbl_mul
};

static const struct guard* guards_op_div[] = {
    &guard_div_by_zero,
    &guard_dbl_and_bignum
};
static const struct op_descriptor op_div = {
    .symbol       = "/",
    .guards       = guards_op_div,
    .guardc       = LENGTH(guards_op_div),
    .neutral      = &lone,
    .op_num       = op_num_div,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_fdiv_q,
    .op_dbl       = op_dbl_div,
};

static const struct guard* guards_op_mod[] = {
    &guard_div_by_zero,
    &guard_either_is_double,
    &guard_dbl_and_bignum,
};
static const struct op_descriptor op_mod = {
    .symbol       = "%",
    .guards       = guards_op_mod,
    .guardc       = LENGTH(guards_op_mod),
    .neutral      = &lone,
    .op_num       = op_num_mod,
    .cnd_overflow = NULL,
    .op_bignum    = mpz_mod,
    .op_dbl       = op_dbl_nop
};

static const struct guard* guards_op_fac[] = {
    &guard_either_is_double,
    &guard_x_is_negative,
    &guard_x_too_big
};
static const struct op_descriptor op_fac = {
    .symbol       = "!",
    .unary        = true,
    .guards       = guards_op_fac,
    .guardc       = LENGTH(guards_op_fac),
    .op_num       = op_num_fact,
    .cnd_overflow = cnd_num_fact_overflow,
    .op_bignum    = op_bignum_fac,
    .op_dbl       = op_dbl_nop
};

static const struct guard *guards_op_pow[] = {
    &guard_dbl_and_bignum,
    &guard_y_is_negative,
    &guard_y_too_big
};
static const struct op_descriptor op_pow = {
    .symbol       = "^",
    .guards       = guards_op_pow,
    .guardc       = LENGTH(guards_op_pow),
    .neutral      = &lzero,
    .op_num       = op_num_pow,
    .cnd_overflow = cnd_num_pow_overflow,
    .op_bignum    = op_bignum_pow,
    .op_dbl       = op_dbl_pow
};

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
