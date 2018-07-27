#include "llexer.h"

#include "vendor/snow/snow/snow.h"

bool test_helper(const char* input, struct ltok* expected) {
    struct ltok *first = NULL, *error = NULL;
    first = lisp_lex(input, &error);
    bool ret = false;
    if (error != NULL || !(ret = llex_are_all_equal(first, expected))) {
        fputs("\nInput:", stdout);
        fputs(input, stdout);
        fputc('\n', stdout);
        fputs("Got:\n", stdout);
        llex_print_all(first);
        fputs("Expected:\n", stdout);
        llex_print_all(expected);
    }
    llex_free(first);
    return ret;
}

describe(llex, {

    it("passes for zero length string", {
        const char* input = "";
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        struct ltok* expected = &tEOF;
        assert(test_helper(input, expected));
    });

    it("fails for unknwon characters", {
        const char* input = "[]";
        struct ltok *first = NULL, *error = NULL;
        first = lisp_lex(input, &error);
        defer(llex_free(first));
        assert(error != NULL);
    });

    it("skip spaces", {
        const char* input = "   \t \r \n   ";
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        struct ltok* expected = &tEOF;
        assert(test_helper(input, expected));
    });

    it("passes for parenthesis", {
        const char* input = "(()";
        struct ltok tok1 = {.type= LTOK_OPAR, .content= "("};
        struct ltok tok2 = {.type= LTOK_OPAR, .content= "("};
        struct ltok tok3 = {.type= LTOK_CPAR, .content= ")"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

    it("passes for integers", {
        const char* input = "1 234  -32";
        struct ltok tok1 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tok2 = {.type= LTOK_NUM, .content= "234"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "-32"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

    it("passes for doubles", {
        const char* input = "0.0 234.0 -32.0";
        struct ltok tok1 = {.type= LTOK_DBL, .content= "0.0"};
        struct ltok tok2 = {.type= LTOK_DBL, .content= "234.0"};
        struct ltok tok3 = {.type= LTOK_DBL, .content= "-32.0"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

    it("passes for strings", {
        const char* input = " \"string1\"  \"string2\"";
        struct ltok tok1 = {.type= LTOK_STR, .content= "\"string1\""};
        struct ltok tok2 = {.type= LTOK_STR, .content= "\"string2\""};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

    it("passes for escaped strings", {
        const char* input = " \"string1\\\"escape\"  \"string2 \\\" with escape and space and number 42.\"";
        struct ltok tok1 = {.type= LTOK_STR, .content= "\"string1\\\"escape\""};
        struct ltok tok2 = {.type= LTOK_STR, .content= "\"string2 \\\" with escape and space and number 42.\""};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

    it("fails for unclosed strings", {
        const char* input = "\"string not closed";
        struct ltok *first = NULL, *error = NULL;
        first = lisp_lex(input, &error);
        defer(llex_free(first));
        llex_printlen(error);
        assert(error != NULL);
    });

    it("passes for symbols", {
        const char* input = " symbol  + == $";
        struct ltok tok1 = {.type= LTOK_SYM, .content="symbol"};
        struct ltok tok2 = {.type= LTOK_SYM, .content="+"};
        struct ltok tok3 = {.type= LTOK_SYM, .content="=="};
        struct ltok tok4 = {.type= LTOK_SYM, .content="$"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tok4;
        tok4.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input, expected));
    });

});

snow_main();
