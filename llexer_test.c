#include "llexer.h"

#include "vendor/snow/snow/snow.h"

bool test_helper(const char* input, struct ltok* expected) {
    struct ltok *first = NULL, *last = NULL;
    first = lisp_lex(input, last);
    bool ret = llex_are_all_equals(first, expected);
    llex_free(first);
    return ret;
}

describe(llex, {

    static const char* input1 = "+ 1 2";
    it(input1, {
        struct ltok tok1 = {.type= LTOK_SYM, .content= "+"};
        struct ltok tok2 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "2"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;
        struct ltok* expected = &tok1;
        assert(test_helper(input1, expected));
    });

});

snow_main();
