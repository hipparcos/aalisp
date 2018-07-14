#include "polish.c" // To access module defined functions.

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include "vendor/minunit/minunit.h"

int tests_run = 0;

/* Tests: start */

struct testcase {
    struct lval x;
    struct lval y;
    char* input;
    struct lval expected;
};

static char* test_op_helper(
    const char* format,
    const struct op_descriptor descriptor,
    struct testcase testcases[],
    unsigned long number_of_cases, bool unary) {

    char* message = (char*)calloc(sizeof(char), 1024);
    struct lval got;
    char temp[4][128];

    for (int i = 0; i < (int)number_of_cases; i++) {
        struct testcase tt = testcases[i];
        got = polish_op(descriptor, tt.x, tt.y);
        lval_to_string(tt.x, temp[0]);
        if (!unary)
            lval_to_string(tt.y, temp[1]);
        lval_to_string(tt.expected, temp[2]);
        lval_to_string(got, temp[3]);
        if (!unary) {
            sprintf(message, format, temp[0], temp[1], temp[2], temp[3]);
        } else {
            sprintf(message, format, temp[0], temp[2], temp[3]);
        }
        mu_assert(message, lval_equals(got, tt.expected) || lval_err_equals(got, tt.expected));
    }

    free(message); // Avoid clang complaint.
    return 0;
}

#define LENGTH(array) sizeof(array)/sizeof(array[0])

const long fac20 = 2432902008176640000;
mpz_t bn_maxlong_succ;
mpz_t bn_minlong_pred;
mpz_t bn_maxlong_x10;
mpz_t bn_fac21;
mpz_t bn_10pow100;

void setup(void) {
    mpz_init_set_si(bn_maxlong_succ, LONG_MAX);
    mpz_add_ui(bn_maxlong_succ, bn_maxlong_succ, 1);

    mpz_init_set_si(bn_minlong_pred, LONG_MIN);
    mpz_sub_ui(bn_minlong_pred, bn_minlong_pred, 1);

    mpz_init_set_si(bn_maxlong_x10, LONG_MAX);
    mpz_mul_si(bn_maxlong_x10, bn_maxlong_x10, 10);

    // fac(20) * 21.
    mpz_init_set_ui(bn_fac21, fac20);
    mpz_mul_si(bn_fac21, bn_fac21, 21);

    mpz_init(bn_10pow100);
    mpz_ui_pow_ui(bn_10pow100, 10, 100);
}

void teardown(void) {
    mpz_clear(bn_maxlong_succ);
    mpz_clear(bn_minlong_pred);
    mpz_clear(bn_maxlong_x10);
    mpz_clear(bn_fac21);
    mpz_clear(bn_10pow100);
}

static char* test_polish_op_add() {
    struct testcase testcases[] = {
        {.x= lval_num(1),   .y= lval_num(1),   .expected= lval_num(2)},
        {.x= lval_num(1),   .y= lval_dbl(1.1), .expected= lval_dbl(2.1)},
        {.x= lval_dbl(1.1), .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
        {.x= lval_num(LONG_MAX),
                            .y= lval_num(1),   .expected= lval_bignum(bn_maxlong_succ)},
        {.x= lval_num(2),   .y= lval_nil(),    .expected= lval_num(2)},
    };
    return test_op_helper("op_add %s + %s = %s got %s", op_add,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_op_sub() {
    struct testcase testcases[] = {
        {.x= lval_num(1),   .y= lval_num(1),   .expected= lval_num(0)},
        {.x= lval_num(1),   .y= lval_dbl(1.1), .expected= lval_dbl(-0.1)},
        {.x= lval_dbl(1.1), .y= lval_dbl(1.1), .expected= lval_dbl(0.0)},
        {.x= lval_num(LONG_MIN),
                            .y= lval_num(1),   .expected= lval_bignum(bn_minlong_pred)},
        {.x= lval_num(2),   .y= lval_nil(),    .expected= lval_num(2)},
    };
    return test_op_helper("op_sub %s - %s = %s got %s", op_sub,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_op_mul() {
    struct testcase testcases[] = {
        {.x= lval_num(2),   .y= lval_num(2),   .expected= lval_num(4)},
        {.x= lval_num(2),   .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
        {.x= lval_dbl(2.0), .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
        {.x= lval_num(LONG_MAX),
                            .y= lval_num(10),  .expected= lval_bignum(bn_maxlong_x10)},
        {.x= lval_num(2),   .y= lval_nil(),    .expected= lval_num(2)},
    };
    return test_op_helper("op_mul %s * %s = %s got %s", op_mul,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_op_div() {
    struct testcase testcases[] = {
        {.x= lval_num(2),   .y= lval_num(2),   .expected= lval_num(1)},
        {.x= lval_num(2),   .y= lval_dbl(2.0), .expected= lval_dbl(1.0)},
        {.x= lval_dbl(2.0), .y= lval_dbl(2.0), .expected= lval_dbl(1.0)},
        {.x= lval_num(2),   .y= lval_num(0),   .expected= lval_err(LERR_DIV_ZERO)},
        {.x= lval_num(2),   .y= lval_dbl(.0),  .expected= lval_err(LERR_DIV_ZERO)},
        {.x= lval_num(2),   .y= lval_nil(),    .expected= lval_num(2)},
    };
    return test_op_helper("op_div %s / %s = %s got %s", op_div,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_op_mod() {
    struct testcase testcases[] = {
        {.x= lval_num(8),   .y= lval_num(3),   .expected= lval_num(2)},
        {.x= lval_num(2),   .y= lval_num(0),   .expected= lval_err(LERR_DIV_ZERO)},
        {.x= lval_num(2),   .y= lval_dbl(2.0), .expected= lval_err(LERR_BAD_NUM)},
        {.x= lval_dbl(2.0), .y= lval_dbl(2.0), .expected= lval_err(LERR_BAD_NUM)},
        {.x= lval_num(2),   .y= lval_nil(),    .expected= lval_num(0)},
    };
    return test_op_helper("op_mod %s %% %s = %s got %s", op_mod,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_op_fac() {
    struct testcase testcases[] = {
        {.x= lval_num(0),  .expected= lval_num(1)},
        {.x= lval_num(1),  .expected= lval_num(1)},
        {.x= lval_num(20), .expected= lval_num(fac20)},
        {.x= lval_num(21), .expected= lval_bignum(bn_fac21)},
        {.x= lval_num(-1), .expected= lval_err(LERR_BAD_NUM)},
    };
    return test_op_helper("op_fact %s! = %s got %s", op_fac,
                          testcases, LENGTH(testcases), true);
}

static char* test_polish_op_pow() {
    struct testcase testcases[] = {
        {.x= lval_num(2),  .y= lval_num(4),   .expected= lval_num(16)},
        {.x= lval_num(4),  .y= lval_dbl(0.5), .expected= lval_dbl(2.0)},
        {.x= lval_num(10), .y= lval_num(100), .expected= lval_bignum(bn_10pow100)},
        {.x= lval_num(2),  .y= lval_num(-1),  .expected= lval_err(LERR_BAD_NUM)},
        {.x= lval_num(2),  .y= lval_nil(),    .expected= lval_num(2)},
    };
    return test_op_helper("op_mod %s %% %s = %s got %s", op_pow,
                          testcases, LENGTH(testcases), false);
}

static char* test_polish_lang() {
    struct testcase testcases[] = {
        {.input= "+ 1 1",          .expected= lval_num(2)},
        {.input= "+ 1 (* 10 2)",   .expected= lval_num(21)},
        {.input= "+ 1.0 (* 10 2)", .expected= lval_dbl(21)},
        {.input= "/ 10 0",         .expected= lval_err(LERR_DIV_ZERO)},
    };

    char* message = (char*)calloc(sizeof(char), 1024);
    char temp[2][128];

    polish_setup();

    for (int i = 0; i < (int)(LENGTH(testcases)); i++) {
        struct testcase tt = testcases[i];
        mpc_result_t r;
        if (mpc_parse("<stdin>", tt.input, Polish, &r)) {
            struct lval got = polish_eval_expr(r.output);
            lval_to_string(tt.expected, temp[0]);
            lval_to_string(got, temp[1]);
            mpc_ast_delete(r.output);
            sprintf(message, "polish_lang: fail for `%s` = %s got %s", tt.input, temp[0], temp[1]);
            mu_assert(message, lval_equals(got, tt.expected) || lval_err_equals(got, tt.expected));
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            sprintf(message, "polish_lang: fail for `%s`", tt.input);
            mu_assert(message, false);
        }
    }

    polish_cleanup();

    free(message); // Avoid clang complaint.
    return 0;
}

static char* all_tests() {
    setup();
    mu_run_test(test_polish_op_add);
    mu_run_test(test_polish_op_sub);
    mu_run_test(test_polish_op_mul);
    mu_run_test(test_polish_op_div);
    mu_run_test(test_polish_op_mod);
    mu_run_test(test_polish_op_fac);
    mu_run_test(test_polish_op_pow);
    mu_run_test(test_polish_lang);
    teardown();
    return 0;
}

/* Tests: end */

int main(void) {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    int code = result != 0;
    if (result) free(result); // Avoid clang complaint.

    return code;
}
