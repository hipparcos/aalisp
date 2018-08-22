#include "lbuiltin.h"

#include "lbuiltin_test.h"

describe(builtin, {

    subdesc(func_head, {
        test_pass(&lbuiltin_head, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 1);
        });
    });

    subdesc(func_tail, {
        test_pass(&lbuiltin_tail, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 2);
            push_num(expected, 3);
        });
    });

    subdesc(func_init, {
        test_pass(&lbuiltin_init, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
        });
    });

    subdesc(func_last, {
        test_pass(&lbuiltin_last, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_cons, {
        test_pass(&lbuiltin_cons, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 1);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
        });

        test_pass(&lbuiltin_cons, "prepending a Q-Expression", {
            struct lval* qexpr1 = lval_alloc();
            lval_mut_qexpr(qexpr1);
            push_num(qexpr1, 2);
            push_num(qexpr1, 3);
            struct lval* qexpr2 = lval_alloc();
            lval_mut_qexpr(qexpr2);
            push_num(qexpr2, 1);
            lval_push(args, qexpr2);
            lval_push(args, qexpr1);
            lval_free(qexpr1);
            lval_mut_qexpr(expected);
            lval_push(expected, qexpr2);
            push_num(expected, 2);
            push_num(expected, 3);
            lval_free(qexpr2);
        });
    });

    subdesc(func_len, {
        test_pass(&lbuiltin_len, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_join, {
        test_pass(&lbuiltin_join, "happy path", {
            struct lval* qexpr1 = lval_alloc();
            lval_mut_qexpr(qexpr1);
            push_num(qexpr1, 1);
            push_num(qexpr1, 2);
            lval_push(args, qexpr1);
            lval_free(qexpr1);
            struct lval* qexpr2 = lval_alloc();
            lval_mut_qexpr(qexpr2);
            push_num(qexpr2, 3);
            push_num(qexpr2, 4);
            lval_push(args, qexpr2);
            lval_free(qexpr2);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
        });

        test_pass(&lbuiltin_join, "no arguments given (returns {})", {
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_list, {
        test_pass(&lbuiltin_list, "happy path", {
            push_num(args, 1);
            push_num(args, 2);
            push_num(args, 3);
            push_num(args, 4);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
        });

        test_pass(&lbuiltin_list, "no arguments given (returns {})", {
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_eval, {
        test_pass(&lbuiltin_eval, "happy path", {
            struct lval* sexpr = lval_alloc();
            lval_mut_sexpr(sexpr);
            push_sym(sexpr, "+");
            push_num(sexpr, 1);
            push_num(sexpr, 2);
            lval_push(args, sexpr);
            lval_free(sexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_def, {
        test_pass(&lbuiltin_def, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_dup(expected, qexpr);
            lval_free(qexpr);
        });

        test_fail(&lbuiltin_def, "symbols > args", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            lval_free(qexpr);
            lval_mut_err_code(expected, LERR_TOO_FEW_ARGS);
        });

        test_fail(&lbuiltin_def, "symbols < args", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_free(qexpr);
            lval_mut_err_code(expected, LERR_TOO_MANY_ARGS);
        });
    });

    subdesc(func_put, {
        test_pass(&lbuiltin_put, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_sym(qexpr, "x");
            push_sym(qexpr, "y");
            lval_push(args, qexpr);
            push_num(args, 100);
            push_num(args, 200);
            lval_dup(expected, qexpr);
            lval_free(qexpr);
        });
    });

    subdesc(func_pack, {
        test_pass(&lbuiltin_pack, "happy path", {
            /* Args. */
            push_func(args, &lbuiltin_head);
            push_num(args, 1);
            push_num(args, 2);
            push_num(args, 3);
            /* Expected. */
            lval_mut_num(expected, 1);
        });
    });

    subdesc(func_unpack, {
        test_pass(&lbuiltin_unpack, "happy path", {
            /* Args. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(qexpr, 4);
            push_func(args, &lbuiltin_op_add);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_num(expected, 10);
        });
    });

});

snow_main();
