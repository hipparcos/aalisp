#include "llexer.h"

#include <string.h>

#include "vendor/snow/snow/snow.h"

struct token_list {
    enum ltok_type type;
    char* content;
};

/** token_list_builder builds singly linked list of tokens from list. */
struct ltok* token_list_builder(const struct token_list* list) {
    struct ltok *head = NULL, **last = &head;
    do {
        struct ltok* curr = calloc(1, sizeof(struct ltok));
        curr->type = list->type;
        size_t len = strlen(list->content);
        curr->content = calloc(1, len + 1);
        strncpy(curr->content, list->content, len);
        *last = curr;
        last = &(curr->next);
    } while (++list && list->content != NULL);
    return head;
}

/** test_pass displays input in header (lisp_lex lexer func). */
#define test_pass(msg, input, expected) \
    it("passes for " msg " `" input "`", { \
        _test_pass_body(input, expected, lisp_lex) \
    })

/** test_pass displays input in header (lisp_lex_surround lexer func). */
#define test_pass_surround(msg, input, expected) \
    it("passes for " msg " `(" input ")` (surrounded input)", { \
        _test_pass_body(input, expected, lisp_lex_surround) \
    })

/** test_pass does not display input in header (lisp_lex lexer func). */
#define test_pass_raw(msg, input, expected) \
    it("passes for " msg, { \
        _test_pass_body(input, expected, lisp_lex) \
    })

#define _test_pass_body(input, expected, lexer) \
        struct ltok* expec = token_list_builder(expected); \
        defer(llex_free(expec)); \
        struct ltok *got = NULL, *error = NULL; \
        defer(llex_free(got)); \
        assert(got = lexer(input, &error)); \
        assert(error == NULL); \
        assert(llex_are_all_equal(got, expec));

#define test_fail(msg, input) \
    it("fails for " msg " `" input "`", { \
        struct ltok *got = NULL, *error = NULL; \
        assert(got = lisp_lex(input, &error)); \
        defer(llex_free(got)); \
        assert(error != NULL); \
    })

describe(llex, {

    test_pass_raw("NULL",
            NULL,
            &((struct token_list[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("0-length",
            "",
            &((struct token_list[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass_raw("spaces",
            "   \t \r \n   ",
            &((struct token_list[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("parenthesis",
            "(()",
            &((struct token_list[]){
                {LTOK_OPAR, "("},
                {LTOK_OPAR, "("},
                {LTOK_CPAR, ")"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("integers",
            "1 234  -32",
            &((struct token_list[]){
                {.type= LTOK_NUM, .content= "1"},
                {.type= LTOK_NUM, .content= "234"},
                {.type= LTOK_NUM, .content= "-32"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("doubles",
            "0.0 234.0  -32.0",
            &((struct token_list[]){
                {LTOK_DBL, "0.0"},
                {LTOK_DBL, "234.0"},
                {LTOK_DBL, "-32.0"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("strings",
            "\"string1\"  \"string2\"",
            &((struct token_list[]){
                {LTOK_STR, "\"string1\""},
                {LTOK_STR, "\"string2\""},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("escaped strings",
            "\"string1\\\"escape\"",
            &((struct token_list[]){
                {LTOK_STR, "\"string1\\\"escape\""},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("symbols",
            "symbol  + == $",
            &((struct token_list[]){
                {LTOK_SYM, "symbol"},
                {LTOK_SYM, "+"},
                {LTOK_SYM, "=="},
                {LTOK_SYM, "$"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("simple expression",
            "+ 1 (! 21)",
            &((struct token_list[]){
                {LTOK_SYM, "+"},
                {LTOK_NUM, "1"},
                {LTOK_OPAR, "("},
                {LTOK_SYM, "!"},
                {LTOK_NUM, "21"},
                {LTOK_CPAR, ")"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass_surround("simple expression",
            "+ 1 (! 21)",
            &((struct token_list[]){
                {LTOK_OPAR, "("},
                {LTOK_SYM, "+"},
                {LTOK_NUM, "1"},
                {LTOK_OPAR, "("},
                {LTOK_SYM, "!"},
                {LTOK_NUM, "21"},
                {LTOK_CPAR, ")"},
                {LTOK_CPAR, ")"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_fail("unknown character", "[]");
    test_fail("unclosed strings", "\"string not closed");

});

snow_main();
