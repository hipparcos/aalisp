#include "polish.c" // To access module defined functions.

#include <stdio.h>
#include "vendor/minunit/minunit.h"

int tests_run = 0;

/* Tests: start */

struct testcase {
    struct lval x;
    struct lval y;
    struct lval expected;
};

static char* test_op_helper(
        const char* format,
        struct lval (*op)(struct lval, struct lval),
        struct testcase testcases[],
        unsigned long number_of_cases) {
    char* message = (char*)calloc(sizeof(char), 1024);
    struct lval got;

    for (int i = 0; i < (int)number_of_cases; i++) {
        struct testcase tt = testcases[i];
        got = op(tt.x, tt.y);
        char temp[4][128];
        lval_to_string(tt.x, temp[0]);
        lval_to_string(tt.y, temp[1]);
        lval_to_string(tt.expected, temp[2]),
        lval_to_string(got, temp[3]);
        sprintf(message, format, temp[0], temp[1], temp[2], temp[3]);
        mu_assert(message, lval_equals(got, tt.expected) || lval_err_equals(got, tt.expected));
    }

    free(message); // Avoid clang complaint.
    return 0;
}

#define LENGTH(array) sizeof(array)/sizeof(array[0])

static char* test_polish_op_add() {
    struct testcase testcases[] = {
        {.x= lval_num(1), .y= lval_num(1), .expected= lval_num(2)},
        {.x= lval_num(1), .y= lval_dbl(1.1), .expected= lval_dbl(2.1)},
        {.x= lval_dbl(1.1), .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
    };
    return test_op_helper("op_add %s + %s = %s got %s", polish_op_add,
            testcases, LENGTH(testcases));
}

static char* test_polish_op_sub() {
    struct testcase testcases[] = {
        {.x= lval_num(1), .y= lval_num(1), .expected= lval_num(0)},
        {.x= lval_num(1), .y= lval_dbl(1.1), .expected= lval_dbl(-0.1)},
        {.x= lval_dbl(1.1), .y= lval_dbl(1.1), .expected= lval_dbl(0.0)},
    };
    return test_op_helper("op_sub %s - %s = %s got %s", polish_op_sub,
            testcases, LENGTH(testcases));
}

static char* test_polish_op_mul() {
    struct testcase testcases[] = {
        {.x= lval_num(2), .y= lval_num(2), .expected= lval_num(4)},
        {.x= lval_num(2), .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
        {.x= lval_dbl(2.0), .y= lval_dbl(1.1), .expected= lval_dbl(2.2)},
    };
    return test_op_helper("op_mul %s * %s = %s got %s", polish_op_mul,
            testcases, LENGTH(testcases));
}

static char* test_polish_op_div() {
    struct testcase testcases[] = {
        {.x= lval_num(2), .y= lval_num(2), .expected= lval_num(1)},
        {.x= lval_num(2), .y= lval_dbl(2.0), .expected= lval_dbl(1.0)},
        {.x= lval_dbl(2.0), .y= lval_dbl(2.0), .expected= lval_dbl(1.0)},
        {.x= lval_num(2), .y= lval_num(0), .expected= lval_err(LERR_DIV_ZERO)},
        {.x= lval_num(2), .y= lval_dbl(.0), .expected= lval_err(LERR_DIV_ZERO)},
    };
    return test_op_helper("op_div %s / %s = %s got %s", polish_op_div,
            testcases, LENGTH(testcases));
}

static char* test_polish_op_mod() {
    struct testcase testcases[] = {
        {.x= lval_num(8), .y= lval_num(3), .expected= lval_num(2)},
        {.x= lval_num(2), .y= lval_num(0), .expected= lval_err(LERR_DIV_ZERO)},
        {.x= lval_num(2), .y= lval_dbl(2.0), .expected= lval_err(LERR_BAD_NUM)},
        {.x= lval_dbl(2.0), .y= lval_dbl(2.0), .expected= lval_err(LERR_BAD_NUM)},
    };
    return test_op_helper("op_mod %s %% %s = %s got %s", polish_op_mod,
            testcases, LENGTH(testcases));
}

static char* all_tests() {
    mu_run_test(test_polish_op_add);
    mu_run_test(test_polish_op_sub);
    mu_run_test(test_polish_op_mul);
    mu_run_test(test_polish_op_div);
    mu_run_test(test_polish_op_mod);
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
