#include "polish.h"

#include <string.h>

#include "vendor/mpc/mpc.h"

/* Private prototypes */
long polish_eval_op(long x, char* op, long y);
long polish_eval_expr(mpc_ast_t* ast);

void polish_eval(const char* restrict input, FILE* output, FILE* error) {
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
        long result = polish_eval_expr(r.output);
        fprintf(output, "%li\n", result);
        mpc_ast_delete(r.output);
    } else {
        mpc_err_print_to(r.error, error);
        mpc_err_delete(r.error);
    }

    mpc_cleanup(4, Number, Operator, Expr, Polish);
}

long polish_eval_expr(mpc_ast_t* ast) {
    if (strstr(ast->tag, "number")) {
        return atol(ast->contents);
    }

    /* Operator is 2nd child. */
    char* op = ast->children[1]->contents;

    /* Eval the remaining children. */
    long x = polish_eval_expr(ast->children[2]);

    for (int i = 3; strstr(ast->children[i]->tag, "expr"); i++) {
        x = polish_eval_op(x, op, polish_eval_expr(ast->children[i]));
    }

    return x;
}

long polish_eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) return x + y;
    if (strcmp(op, "-") == 0) return x - y;
    if (strcmp(op, "*") == 0) return x * y;
    if (strcmp(op, "/") == 0) return x / y;
    return 0;
}
