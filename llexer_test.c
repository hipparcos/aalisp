#include "llexer.h"

#include <string.h>

#include "vendor/snow/snow/snow.h"

/** token_list_builder builds singly linked list of tokens from list. */
struct ltok* token_list_builder(const struct ltok* list) {
    struct ltok *head = NULL, **last = &head;
    do {
        struct ltok* curr = calloc(1, sizeof(struct ltok));
        curr->type = list->type;
        size_t len = strlen(list->content);
        curr->content = malloc(len+1);
        strncpy(curr->content, list->content, len+1);
        *last = curr;
        last = &(curr->next);
    } while (++list && list->content != NULL);
    return head;
}

/** test_pass displays input in header. */
#define test_pass(msg, input, expected) \
    it("pass for " msg, { \
        struct ltok* expec = token_list_builder(expected); \
        defer(llex_free(expec)); \
        struct lerr* err = NULL; \
        defer(lerr_free(err)); \
        struct ltok* got = NULL; \
        defer(llex_free(got)); \
        assert(got = llex_lex(input, &err)); \
        assert(err == NULL); \
        if (!llex_are_all_equal(got, expec)) { \
            puts("\ngot:"); \
            llex_print_all(got); \
            puts("\nexpected:"); \
            llex_print_all(expected); \
            fail("got != expected"); \
        } \
    })


#define test_fail(msg, input) \
    it("fail for " msg, { \
        struct lerr* err = NULL; \
        defer(lerr_free(err)); \
        struct ltok* got = NULL; \
        assert(got = llex_lex(input, &err)); \
        defer(llex_free(got)); \
        assert(err != NULL); \
    })

describe(llex, {

    test_pass("NULL",
            NULL,
            &((struct ltok[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("0-length",
            "",
            &((struct ltok[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("spaces",
            "   \t \r \n   ",
            &((struct ltok[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("comment",
            "; I'm a comment.",
            &((struct ltok[]){
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("parenthesis",
            "(()",
            &((struct ltok[]){
                {LTOK_OPAR, "("},
                {LTOK_OPAR, "("},
                {LTOK_CPAR, ")"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("braces",
            "{{}",
            &((struct ltok[]){
                {LTOK_OBRC, "{"},
                {LTOK_OBRC, "{"},
                {LTOK_CBRC, "}"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("integers",
            "1 234  -32",
            &((struct ltok[]){
                {LTOK_NUM, "1"},
                {LTOK_NUM, "234"},
                {LTOK_NUM, "-32"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("doubles",
            "0.0 234.0  -32.0",
            &((struct ltok[]){
                {LTOK_DBL, "0.0"},
                {LTOK_DBL, "234.0"},
                {LTOK_DBL, "-32.0"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("strings",
            "\"string1\"  \"string2\"",
            &((struct ltok[]){
                {LTOK_STR, "\"string1\""},
                {LTOK_STR, "\"string2\""},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("UTF-8 strings",
            "\"عربى\"  \"भारतीय\"",
            &((struct ltok[]){
                {LTOK_STR, "\"عربى\""},
                {LTOK_STR, "\"भारतीय\""},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("escaped strings",
            "\"string1\\\"escape\"",
            &((struct ltok[]){
                {LTOK_STR, "\"string1\\\"escape\""},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("symbols",
            "symbol  + == -",
            &((struct ltok[]){
                {LTOK_SYM, "symbol"},
                {LTOK_SYM, "+"},
                {LTOK_SYM, "=="},
                {LTOK_SYM, "-"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("simple expression",
            "+ 1 (! 21)",
            &((struct ltok[]){
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

    test_pass("simple expression with $",
            "+ 1 $ ! 21",
            &((struct ltok[]){
                {LTOK_SYM, "+"},
                {LTOK_NUM, "1"},
                {LTOK_DOLL, "$"},
                {LTOK_SYM, "!"},
                {LTOK_NUM, "21"},
                {LTOK_EOF, ""},
                {0, NULL}
            })[0]
        );

    test_pass("multiline expression with comment",
            "+ 1 (! ; I'm a comment.\n21)",
            &((struct ltok[]){
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

    test_fail("unknown character", "[]");
    test_fail("unclosed strings", "\"string not closed");

});

snow_main();
