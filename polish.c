#include "polish.h"

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "vendor/mpc/mpc.h"

enum ltype {
    LVAL_NUM,
    LVAL_DBL,
    LVAL_ERR
};

enum lerr {
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
};

/* lval is the return type of an evalution. */
struct lval {
    enum ltype type;
    union {
        long num;
        double dbl;
        enum lerr err;
    } data;
};

/* Create a new number type lval. */
struct lval lval_num(long x) {
    struct lval v;
    v.type = LVAL_NUM;
    v.data.num = x;
    return v;
}

/* Create a new double type lval. */
struct lval lval_dbl(double x) {
    struct lval v;
    v.type = LVAL_DBL;
    v.data.dbl = x;
    return v;
}

/* Create a new error type lval. */
struct lval lval_err(enum lerr err) {
    struct lval v;
    v.type = LVAL_ERR;
    v.data.err = err;
    return v;
}

bool lval_is_zero(struct lval v) {
    return (v.type == LVAL_NUM && v.data.num == 0)
        || (v.type == LVAL_DBL && fpclassify(v.data.dbl) == FP_ZERO);
}

double lval_as_dbl(struct lval v) {
    switch (v.type) {
    case LVAL_NUM: return (double) v.data.num;
    case LVAL_DBL: return v.data.dbl;
    default: return .0;
    }
}

void lval_print_to(struct lval v, FILE* output) {
    switch (v.type) {
    case LVAL_NUM:
        fprintf(output, "%li", v.data.num);
        break;
    case LVAL_DBL:
        fprintf(output, "%g", v.data.dbl);
        break;
    case LVAL_ERR:
        switch (v.data.err) {
        case LERR_DIV_ZERO:
            fprintf(output, "Error: division by 0.");
            break;
        case LERR_BAD_OP:
            fprintf(output, "Error: invalid operator.");
            break;
        case LERR_BAD_NUM:
            fprintf(output, "Error: invalid number.");
            break;
        }
        break;
    }
}

void lval_print(struct lval v)   { lval_print_to(v, stdout); }
void lval_println(struct lval v) { lval_print_to(v, stdout); putchar('\n'); }

/* Private prototypes */
struct lval polish_eval_op(struct lval x, char* op, struct lval y);
struct lval polish_eval_expr(mpc_ast_t* ast);

void polish_eval(const char* restrict input) {
    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Double    = mpc_new("double");
    mpc_parser_t* Operator  = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Polish    = mpc_new("polish");

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

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Polish, &r)) {
        struct lval result = polish_eval_expr(r.output);
        lval_println(result);
        mpc_ast_delete(r.output);
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }

    mpc_cleanup(5, Number, Double, Operator, Expr, Polish);
}

struct lval polish_eval_expr(mpc_ast_t* ast) {
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

#define EITHER_IS_DBL(x,y) x.type == LVAL_DBL || y.type == LVAL_DBL

struct lval polish_op_add(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) + lval_as_dbl(y));
    } else {
        return lval_num(x.data.num + y.data.num);
    }
}

struct lval polish_op_sub(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) - lval_as_dbl(y));
    } else {
        return lval_num(x.data.num - y.data.num);
    }
}

struct lval polish_op_mul(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) * lval_as_dbl(y));
    } else {
        return lval_num(x.data.num * y.data.num);
    }
}

struct lval polish_op_div(struct lval x, struct lval y) {
    if (lval_is_zero(y)) {
        return lval_err(LERR_DIV_ZERO);
    }

    if (EITHER_IS_DBL(x,y)) {
        return lval_dbl(lval_as_dbl(x) / lval_as_dbl(y));
    } else {
        return lval_num(x.data.num / y.data.num);
    }
}

struct lval polish_op_mod(struct lval x, struct lval y) {
    if (EITHER_IS_DBL(x, y)) {
        return lval_err(LERR_BAD_NUM);
    }

    if (lval_is_zero(y)) {
        return lval_err(LERR_DIV_ZERO);
    }

    return lval_num(x.data.num % y.data.num);
}

struct lval polish_eval_op(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return polish_op_add(x, y);
    if (strcmp(op, "-") == 0) return polish_op_sub(x, y);
    if (strcmp(op, "*") == 0) return polish_op_mul(x, y);
    if (strcmp(op, "/") == 0) return polish_op_div(x, y);
    if (strcmp(op, "%") == 0) return polish_op_mod(x, y);

    return lval_err(LERR_BAD_NUM);
}
