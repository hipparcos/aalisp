#include "polish.h"

#include <string.h>

#include "vendor/mpc/mpc.h"

enum ltype {
    LVAL_NUM,
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
    long num;
    enum lerr err;
};

/* Create a new number type lval. */
struct lval lval_num(long x) {
    struct lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

/* Create a new error type lval. */
struct lval lval_err(enum lerr err) {
    struct lval v;
    v.type = LVAL_ERR;
    v.err = err;
    return v;
}

void lval_print_to(struct lval v, FILE* output) {
    switch (v.type) {
    case LVAL_NUM:
        fprintf(output, "%li", v.num);
        break;
    case LVAL_ERR:
        switch (v.err) {
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
    mpc_parser_t* Operator  = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Polish    = mpc_new("polish");

    mpca_lang(MPCA_LANG_DEFAULT,
              "                                                  \
              number   : /-?[0-9]+/ ;                            \
              operator : '+' | '-' | '*' | '/' ;                 \
              expr     : <number> | '(' <operator> <expr>+ ')' ; \
              polish   : /^/ <operator> <expr>+ /$/ ;            \
              ",
              Number, Operator, Expr, Polish);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Polish, &r)) {
        struct lval result = polish_eval_expr(r.output);
        lval_println(result);
        mpc_ast_delete(r.output);
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
    }

    mpc_cleanup(4, Number, Operator, Expr, Polish);
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

    /* Operator is 2nd child. */
    char* op = ast->children[1]->contents;

    /* Eval the remaining children. */
    struct lval x = polish_eval_expr(ast->children[2]);

    for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
        x = polish_eval_op(x, op, polish_eval_expr(ast->children[i]));
    }

    return x;
}

struct lval polish_eval_op(struct lval x, char* op, struct lval y) {
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0) return lval_num(x.num + y.num);
    if (strcmp(op, "-") == 0) return lval_num(x.num - y.num);
    if (strcmp(op, "*") == 0) return lval_num(x.num * y.num);
    if (strcmp(op, "/") == 0) {
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    }

    return lval_err(LERR_BAD_NUM);
}
