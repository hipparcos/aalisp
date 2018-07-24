#include "lparser.h"

#include <stdbool.h>

#include "llexer.h"

#include "vendor/snow/snow/snow.h"

bool test_helper_pass(struct ltok* input, const struct last* expected) {
    struct last* got = NULL;
    struct last* error = NULL;
    got = lisp_parse(input, &error);
    if (!got || error != NULL || !last_are_all_equal(got, expected)) {
        fputc('\n', stdout);
        fputs("Input:\n", stdout);
        llex_print_all(input);
        fputs("Expected:\n", stdout);
        last_print_all(expected);
        fputs("Got:\n", stdout);
        last_print_all(got);
        fputs("Error:\n", stdout);
        last_print(error);
        fputc('\n', stdout);
        last_free(got);
        return false;
    }
    last_free(got);
    return true;
}
bool test_helper_fail(struct ltok* input) {
    struct last* got = NULL;
    struct last* error = NULL;
    got = lisp_parse(input, &error);
    if (!got || error == NULL) {
        fputc('\n', stdout);
        fputs("Input:\n", stdout);
        llex_print_all(input);
        fputs("Got:\n", stdout);
        last_print_all(got);
        fputs("Error:\n", stdout);
        last_print(error);
        fputc('\n', stdout);
        last_free(got);
        return false;
    }
    last_free(got);
    return true;
}

describe(lparse, {

    it("works for 0 length token list", {
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        struct last* error = NULL;
        assert(lisp_parse(&tEOF, &error) == NULL);
    });

    it("works for expression `+ 1 2`", {
        /* Input */
        struct ltok tok1 = {.type= LTOK_SYM, .content= "+"};
        struct ltok tok2 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "2"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;

        /* Expected */
        struct last symb = {.tag= LTAG_SYM, .content= "+"};
        struct last opr1 = {.tag= LTAG_NUM, .content= "1"};
        struct last opr2 = {.tag= LTAG_NUM, .content= "2"};
        struct last *expr_children[] = { &symb, &opr1, &opr2 };
        struct last expr = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr_children,
            .childrenc = 3,
        };
        struct last *prgm_children[] = { &expr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(&tok1, &prgm));
    });

    it("works for double", {
        /* Input */
        struct ltok tok1 = {.type= LTOK_SYM, .content= "+"};
        struct ltok tok2 = {.type= LTOK_DBL, .content= "1.0"};
        struct ltok tok3 = {.type= LTOK_DBL, .content= "2.0"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;

        /* Expected */
        struct last symb = {.tag= LTAG_SYM, .content= "+"};
        struct last opr1 = {.tag= LTAG_DBL, .content= "1.0"};
        struct last opr2 = {.tag= LTAG_DBL, .content= "2.0"};
        struct last *expr_children[] = { &symb, &opr1, &opr2 };
        struct last expr = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr_children,
            .childrenc = 3,
        };
        struct last *prgm_children[] = { &expr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(&tok1, &prgm));
    });

    it("fails for an expression which does not begin by a symbol `1 + 2`", {
        /* Input */
        struct ltok tok1 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tok2 = {.type= LTOK_SYM, .content= "+"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "2"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;

        assert(test_helper_fail(&tok1));
    });

    it("fails for a s-expression which does not end by a `)`", {
        /* Input */
        struct ltok tok1 = {.type= LTOK_OPAR, .content= "("};
        struct ltok tok2 = {.type= LTOK_SYM, .content= "!"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;

        assert(test_helper_fail(&tok1));
    });

});

snow_main();
