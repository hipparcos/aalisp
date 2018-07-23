#include "lparser.h"

#include "vendor/snow/snow/snow.h"

describe(lparse, {

    it("tests parsing", {
        fputc('\n', stdout);
        
        struct ltok tok1 = {.type= LTOK_SYM, .content= "+"};
        struct ltok tok2 = {.type= LTOK_NUM, .content= "1"};
        struct ltok tok3 = {.type= LTOK_NUM, .content= "2"};
        struct ltok tEOF = {.type= LTOK_EOF, .content= NULL};
        tok1.next = &tok2;
        tok2.next = &tok3;
        tok3.next = &tEOF;

        struct last* ast = NULL;
        assert(ast = lisp_parse(&tok1));

        fputc('\n', stdout);
        llex_print_all(&tok1);
        fputc('\n', stdout);
        last_print_all(ast);
        fputc('\n', stdout);

        last_free(ast);
    });

});

snow_main();
