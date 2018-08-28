#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leval.h"
#include "lerr.h"
#include "lenv.h"

#define PROMPT_STR ">>>>>>>> "
#define EXPEC_STR  "expected "
#define GOT_STR    "got      "

void test_marker(const char* restrict input, size_t expected) {
    fprintf(stderr, "%s%s\n", PROMPT_STR, input);
    fprintf(stderr, "%s", EXPEC_STR);
    struct lerr* err_expec = lerr_alloc();
    err_expec->col = expected;
    lerr_print_marker_to(err_expec, 0, stderr);
    lerr_free(err_expec);
    fprintf(stderr, "%s", GOT_STR);
    struct lenv* env = lenv_alloc();
    lenv_default(env);
    struct lerr* err = lisp_eval_from_string(env, input);
    if (err) {
        lerr_set_file(err, "test");
        lerr_print_marker_to(err, 0, stderr);
        lerr_print_to(err, stderr);
        lerr_free(err);
    }
    lenv_free(env);
}

int main(void) {
    fputs("Error marker tests:\n", stderr);
    fputs("===================\n", stderr);
    /* Lexer errors. */
    test_marker("+ 1 \"tes",    5);
    /* Parser errors. */
    test_marker("+ 1 (+ 1",     9);
    test_marker("+ 1 {1 1",     9);
    test_marker("+ 1 (1 1)",    6);
    /* Evaluation errors. */
    test_marker("+ 1 \"test\"", 5);
    test_marker("!",            1);
    test_marker("! 4 4",        1);
    test_marker("join 1 {2 3}", 6);
    test_marker("+ (!1)",       4);
    test_marker("+ (! \"t\")",  6);
    return EXIT_SUCCESS;
}
