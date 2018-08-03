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

    it("works for s-expression `(+ 1 2)`", {
        /* Input */
        const char* lexer_input = "(+ 1 2)";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

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
        struct last *sexpr_children[] = { &expr };
        struct last sexpr = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr_children,
            .childrenc = 1,
        };
        struct last *prgm_children[] = { &sexpr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(tokens, &prgm));
    });

    it("works for expression `(+ 1 +)`", {
        /* Input */
        const char* lexer_input = "(+ 1 +)";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        /* Expected */
        struct last symb = {.tag= LTAG_SYM, .content= "+"};
        struct last opr1 = {.tag= LTAG_NUM, .content= "1"};
        struct last opr2 = {.tag= LTAG_SYM, .content= "+"};
        struct last *expr_children[] = { &symb, &opr1, &opr2 };
        struct last expr = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr_children,
            .childrenc = 3,
        };
        struct last *sexpr_children[] = { &expr };
        struct last sexpr = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr_children,
            .childrenc = 1,
        };
        struct last *prgm_children[] = { &sexpr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(tokens, &prgm));
    });

    it("works for double", {
        /* Input */
        const char* lexer_input = "(+ 1.0 2.0)";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

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
        struct last *sexpr_children[] = { &expr };
        struct last sexpr = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr_children,
            .childrenc = 1,
        };
        struct last *prgm_children[] = { &sexpr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(tokens, &prgm));
    });

    it("works for strings", {
        /* Input */
        const char* lexer_input = "(concat \"test\" \"esc\\\"aped\")";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        /* Expected */
        struct last symb = {.tag= LTAG_SYM, .content= "concat"};
        struct last opr1 = {.tag= LTAG_STR, .content= "test"};
        struct last opr2 = {.tag= LTAG_STR, .content= "esc\"aped"};
        struct last *expr_children[] = { &symb, &opr1, &opr2 };
        struct last expr = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr_children,
            .childrenc = 3,
        };
        struct last *sexpr_children[] = { &expr };
        struct last sexpr = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr_children,
            .childrenc = 1,
        };
        struct last *prgm_children[] = { &sexpr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(tokens, &prgm));
    });

    it("works for nested s-expressions `(+ 1 (! 2))`", {
        /* Input */
        const char* lexer_input = "(+ 1 (! 2))";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        /* Expected */
        struct last symb2 = {.tag= LTAG_SYM, .content= "!"};
        struct last opr2 = {.tag= LTAG_NUM, .content= "2"};
        struct last *expr3_children[] = { &symb2, &opr2 };
        struct last expr3 = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr3_children,
            .childrenc = 2,
        };
        struct last *sexpr2_children[] = { &expr3 };
        struct last sexpr2 = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr2_children,
            .childrenc = 1,
        };
        struct last symb1 = {.tag= LTAG_SYM, .content= "+"};
        struct last opr1 = {.tag= LTAG_NUM, .content= "1"};
        struct last *expr1_children[] = { &symb1, &opr1, &sexpr2 };
        struct last expr1 = {
            .tag= LTAG_EXPR, .content= "",
            .children= (struct last**) &expr1_children,
            .childrenc = 3,
        };
        struct last *sexpr_children[] = { &expr1 };
        struct last sexpr = {
            .tag= LTAG_SEXPR, .content= "",
            .children= (struct last**) &sexpr_children,
            .childrenc = 1,
        };
        struct last *prgm_children[] = { &sexpr };
        struct last prgm = {
            .tag= LTAG_PROG, .content= "",
            .children= (struct last**) &prgm_children,
            .childrenc = 1,
        };

        assert(test_helper_pass(tokens, &prgm));
    });

    it("fails for a s-expression which does not begin by a symbol `(1 + 2)`", {
        /* Input */
        const char* lexer_input = "(1 + 2)";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        assert(test_helper_fail(tokens));
    });

    it("fails for a s-expression which does not end by a `)`", {
        /* Input */
        const char* lexer_input = "(! 1";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        assert(test_helper_fail(tokens));
    });

    it("fails for a s-expression that starts right before the end of a line `- (`", {
        /* Input */
        const char* lexer_input = "(";
        struct ltok* lexer_error = NULL;
        struct ltok* tokens = lisp_lex(lexer_input, &lexer_error);
        defer(llex_free(tokens));

        assert(test_helper_fail(tokens));
    });

});

snow_main();
