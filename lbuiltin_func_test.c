#include "lbuiltin.h"

#include "lbuiltin_test.h"

describe(builtin, {

    subdesc(func_if, {
        test_pass(&lbuiltin_if, "happy path branch true", {
            push_bool(args, true);
            push_num(args, 1);
            push_num(args, 2);
            lval_mut_num(expected, 1);
        });
        test_pass(&lbuiltin_if, "happy path branch false", {
            push_bool(args, false);
            push_num(args, 1);
            push_num(args, 2);
            lval_mut_num(expected, 2);
        });
    });

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

    subdesc(func_index, {
        test_pass(&lbuiltin_index, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 1);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 2);
        });
        test_pass(&lbuiltin_index, "got nil for index > len", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 3);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_nil(expected);
        });
        test_pass(&lbuiltin_index, "indexed from last element for index < 0", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, -1);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_num(expected, 3);
        });
    });

    subdesc(func_take, {
        test_pass(&lbuiltin_take, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 2);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
        });
        test_pass(&lbuiltin_take, "len take < 0; take from last", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, -2);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 2);
            push_num(expected, 3);
        });
        test_pass(&lbuiltin_take, "take > len", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 4);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
        });
        test_pass(&lbuiltin_take, "take < 0 && abs(take) > len", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, -4);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
        });
        test_pass(&lbuiltin_take, "take 0", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 0);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
        });
        test_pass(&lbuiltin_take, "take for strings", {
            push_num(args, 2);
            push_str(args, "abcd");
            lval_mut_str(expected, "ab");
        });
    });

    subdesc(func_elem, {
        test_pass(&lbuiltin_elem, "elem in list", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 2);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_bool(expected, true);
        });
        test_pass(&lbuiltin_elem, "elem not in list", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 4);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_bool(expected, false);
        });
        test_pass(&lbuiltin_elem, "elem in string", {
            push_str(args, "d");
            push_str(args, "abcd");
            lval_mut_bool(expected, true);
        });
        test_pass(&lbuiltin_elem, "elem not in string", {
            push_str(args, "x");
            push_str(args, "abcd");
            lval_mut_bool(expected, false);
        });
    });

    subdesc(func_drop, {
        test_pass(&lbuiltin_drop, "happy path", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 2);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 3);
        });
        test_pass(&lbuiltin_drop, "drop from last element for index < 0", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, -2);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
        });
        test_pass(&lbuiltin_drop, "drop 0", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 0);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
        });
        test_pass(&lbuiltin_drop, "got nil for index > len", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, 4);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
        });
        test_pass(&lbuiltin_drop, "got nil for -index > len", {
            struct lval* qexpr = lval_alloc();
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(args, -4);
            lval_push(args, qexpr);
            lval_free(qexpr);
            lval_mut_qexpr(expected);
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

    subdesc(func_seq, {
        test_pass(&lbuiltin_seq, "seq 1 1", {
            push_num(args, 1);
            push_num(args, 1);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
        });
        test_pass(&lbuiltin_seq, "seq 1 5", {
            push_num(args, 1);
            push_num(args, 5);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
            push_num(expected, 5);
        });
        test_pass(&lbuiltin_seq, "seq 1 5 2", {
            push_num(args, 1);
            push_num(args, 5);
            push_num(args, 2);
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 3);
            push_num(expected, 5);
        });
        test_pass(&lbuiltin_seq, "seq -1 -5", {
            push_num(args, -1);
            push_num(args, -5);
            lval_mut_qexpr(expected);
            push_num(expected, -1);
            push_num(expected, -2);
            push_num(expected, -3);
            push_num(expected, -4);
            push_num(expected, -5);
        });
        test_pass(&lbuiltin_seq, "seq -1 -5 -2", {
            push_num(args, -1);
            push_num(args, -5);
            push_num(args, -2);
            lval_mut_qexpr(expected);
            push_num(expected, -1);
            push_num(expected, -3);
            push_num(expected, -5);
        });
    });

    subdesc(func_mix, {
        test_pass(&lbuiltin_mix, "mix {}", {
            struct lval* list = lval_alloc();
            defer(lval_free(list));
            lval_mut_qexpr(list);
            lval_push(args, list);
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_filter, {
        test_pass(&lbuiltin_filter, "happy path", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_lt);
            struct lfunc* fun = lval_as_func(func);
            push_num(fun->args, 2);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(qexpr, 4);
            /* Args. */
            lval_push(args, func);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_qexpr(expected);
            push_num(expected, 3);
            push_num(expected, 4);
        });
    });

    subdesc(func_fold, {
        test_pass(&lbuiltin_fold, "happy path", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_add);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(qexpr, 4);
            /* Args. */
            lval_push(args, func);
            push_num(args, 0);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_num(expected, 10);
        });
    });

    subdesc(func_reverse, {
        test_pass(&lbuiltin_reverse, "happy path Q-Expression", {
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            push_num(qexpr, 3);
            push_num(qexpr, 4);
            /* Args. */
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_qexpr(expected);
            push_num(expected, 4);
            push_num(expected, 3);
            push_num(expected, 2);
            push_num(expected, 1);
        });
        test_pass(&lbuiltin_reverse, "happy path string", {
            /* List. */
            struct lval* str = lval_alloc();
            defer(lval_free(str));
            lval_mut_str(str, "input");
            /* Args. */
            lval_push(args, str);
            /* Expected. */
            lval_mut_str(expected, "tupni");
        });
    });

    subdesc(func_all, {
        test_pass(&lbuiltin_all, "return true for (< 10) {100 110 120}", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_lt);
            struct lfunc* fun = lval_as_func(func);
            push_num(fun->args, 10);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 100);
            push_num(qexpr, 110);
            push_num(qexpr, 120);
            /* Args. */
            lval_push(args, func);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_bool(expected, true);
        });
        test_pass(&lbuiltin_all, "return false for (< 10) {100 1 120}", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_lt);
            struct lfunc* fun = lval_as_func(func);
            push_num(fun->args, 10);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 100);
            push_num(qexpr, 1);
            push_num(qexpr, 120);
            /* Args. */
            lval_push(args, func);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_bool(expected, false);
        });
    });

    subdesc(func_any, {
        test_pass(&lbuiltin_any, "return true for (< 10) {100 1 2}", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_lt);
            struct lfunc* fun = lval_as_func(func);
            push_num(fun->args, 10);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 100);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            /* Args. */
            lval_push(args, func);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_bool(expected, true);
        });
        test_pass(&lbuiltin_any, "return false for (< 10) {0 1 2}", {
            /* Function. */
            struct lval* func = lval_alloc();
            defer(lval_free(func));
            lval_mut_func(func, &lbuiltin_op_lt);
            struct lfunc* fun = lval_as_func(func);
            push_num(fun->args, 10);
            /* List. */
            struct lval* qexpr = lval_alloc();
            defer(lval_free(qexpr));
            lval_mut_qexpr(qexpr);
            push_num(qexpr, 0);
            push_num(qexpr, 1);
            push_num(qexpr, 2);
            /* Args. */
            lval_push(args, func);
            lval_push(args, qexpr);
            /* Expected. */
            lval_mut_bool(expected, false);
        });
    });

    subdesc(func_zip, {
        test_pass(&lbuiltin_zip, "Q-Expression len0 == len1", {
            /* List. */
            struct lval* list0 = lval_alloc();
            defer(lval_free(list0));
            lval_mut_qexpr(list0);
            push_num(list0, 1);
            push_num(list0, 2);
            push_num(list0, 3);
            struct lval* list1 = lval_alloc();
            defer(lval_free(list1));
            lval_mut_qexpr(list1);
            push_str(list1, "a");
            push_str(list1, "b");
            push_str(list1, "c");
            /* Args. */
            lval_push(args, list0);
            lval_push(args, list1);
            /* Expected. */
            lval_mut_qexpr(expected);
            long key[3] = {1, 2, 3};
            char val[3][2] = {"a", "b", "c"};
            for (size_t e = 0; e < 3; e++) {
                struct lval* zipped = lval_alloc();
                lval_mut_qexpr(zipped);
                push_num(zipped, key[e]);
                push_str(zipped, val[e]);
                lval_push(expected, zipped);
                lval_free(zipped);
            }
        });
    });

    subdesc(func_sort, {
        test_pass(&lbuiltin_sort, "Q-Expression", {
            /* List. */
            struct lval* list = lval_alloc();
            defer(lval_free(list));
            lval_mut_qexpr(list);
            push_num(list, 5);
            push_num(list, 3);
            push_num(list, 2);
            push_num(list, 4);
            push_num(list, 1);
            /* Args. */
            lval_push(args, list);
            /* Expected. */
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 2);
            push_num(expected, 3);
            push_num(expected, 4);
            push_num(expected, 5);
        });
        test_pass(&lbuiltin_sort, "empty list", {
            /* List. */
            struct lval* list = lval_alloc();
            defer(lval_free(list));
            lval_mut_qexpr(list);
            /* Args. */
            lval_push(args, list);
            /* Expected. */
            lval_mut_qexpr(expected);
        });
    });

    subdesc(func_repeat, {
        test_pass(&lbuiltin_repeat, "repeat 3 {1}", {
            /* List. */
            struct lval* list = lval_alloc();
            defer(lval_free(list));
            lval_mut_qexpr(list);
            push_num(list, 1);
            /* Args. */
            push_num(args, 3);
            lval_push(args, list);
            /* Expected. */
            lval_mut_qexpr(expected);
            push_num(expected, 1);
            push_num(expected, 1);
            push_num(expected, 1);
        });
        test_pass(&lbuiltin_repeat, "empty list", {
            /* List. */
            struct lval* list = lval_alloc();
            defer(lval_free(list));
            lval_mut_qexpr(list);
            /* Args. */
            push_num(args, 3);
            lval_push(args, list);
            /* Expected. */
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
