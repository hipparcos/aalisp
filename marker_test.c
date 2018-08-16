#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "leval.h"

#define PROMPT_STR ">>>>>>>> "
#define EXPEC_STR  "expected "
#define GOT_STR    "got      "

extern void print_error_marker(FILE* out, int indent, int col);

void test_marker(const char* restrict input, size_t expected) {
    fprintf(stderr, "%s%s\n", PROMPT_STR, input);
    fprintf(stderr, "%s", EXPEC_STR);
    print_error_marker(stdout, 0, expected);
    fprintf(stderr, "%s", GOT_STR);
    lisp_eval_from_string(input, 0);
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
