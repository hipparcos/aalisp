#include "lisp.c" // To access module defined functions.

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

#define LENGTH(array) sizeof(array)/sizeof(array[0])

void setup(void) {
    //...
}

void teardown(void) {
    //...
}

static char* test_lisp_lang() {
    struct testcase testcases[] = {
        {.input= "+ 1 1",          .expected= lval_num(2)},
        {.input= "+ 1 (* 10 2)",   .expected= lval_num(21)},
        {.input= "+ 1.0 (* 10 2)", .expected= lval_dbl(21)},
        {.input= "/ 10 0",         .expected= lval_err(LERR_DIV_ZERO)},
    };

    char* message = (char*)calloc(sizeof(char), 1024);
    char temp[2][128];

    lisp_setup();

    for (int i = 0; i < (int)(LENGTH(testcases)); i++) {
        struct testcase tt = testcases[i];
        mpc_result_t r;
        if (mpc_parse("<stdin>", tt.input, Lisp, &r)) {
            struct lval got = lisp_eval_expr(r.output);
            lval_to_string(tt.expected, temp[0]);
            lval_to_string(got, temp[1]);
            mpc_ast_delete(r.output);
            sprintf(message, "lisp_lang: fail for `%s` = %s got %s", tt.input, temp[0], temp[1]);
            mu_assert(message, lval_equals(got, tt.expected) || lval_err_equals(got, tt.expected));
            lval_clear(&got);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            sprintf(message, "lisp_lang: fail for `%s`", tt.input);
            mu_assert(message, false);
        }
    }

    lisp_teardown();

    free(message); // Avoid clang complaint.
    return 0;
}

static char* all_tests() {
    setup();
    mu_run_test(test_lisp_lang);
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
