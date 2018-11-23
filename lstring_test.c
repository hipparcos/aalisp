#include "lstring.h"

#include <string.h>

#include "vendor/snow/snow/snow.h"

describe(lstr, {
    it("creates a lstr", {
        const char* input = "test";
        struct lstr* s = NULL;
        defer(lstr_free(s));
        assert(s = lstr(input));
        assert(strcmp(s->content, input) == 0);
    });

    it("creates a lstr with embedded '0'", {
        const char* input = "test\0test";
        size_t input_len = 9;
        struct lstr* s = NULL;
        defer(lstr_free(s));
        assert(s = lstrn(input, input_len));
        assert(memcmp(s->content, input, input_len) == 0);
    });

    subdesc(lstr_cat, {
        it("concatenates two lstr", {
            #define INPUT1 "str1" // easier to concatenate.
            #define INPUT2 "str2" // easier to concatenate.
            struct lstr* s1 = lstr(INPUT1);
            defer(lstr_free(s1));
            struct lstr* s2 = lstr(INPUT2);
            defer(lstr_free(s2));
            struct lstr* buf = NULL;
            defer(lstr_free(buf));
            assert(buf = lbuf(s1->len + s2->len));
            assert(lstr_cat(buf, s1));
            assert(lstr_cat(buf, s2));
            assert(strcmp(buf->content, INPUT1 INPUT2) == 0);
        });

        it("handles NULL gracefully", {
            struct lstr* s = lstr("test");
            defer(lstr_free(s));
            struct lstr* buf = lbuf(1);
            defer(lstr_free(buf));
            assert(lstr_cat(NULL, NULL) == NULL);
            assert(lstr_cat(NULL, s) == NULL);
            assert(lstr_cat(buf, NULL) == NULL);
        });

        it("fails is buf have not enough cap to concatenate src", {
            struct lstr* s = lstr("test");
            defer(lstr_free(s));
            struct lstr* buf = lbuf(1);
            defer(lstr_free(buf));
            assert(lstr_cat(buf, s) == NULL);
            assert(strcmp(buf->content, "") == 0);
        });
    });

    subdesc(lstr_cat_cstr, {
        it("concatenates two NULL terminated strings", {
            #define INPUT1 "str1" // easier to concatenate.
            #define INPUT2 "str2" // easier to concatenate.
            struct lstr* buf = NULL;
            defer(lstr_free(buf));
            assert(buf = lbuf(strlen(INPUT1) + strlen(INPUT2)));
            assert(lstr_cat_cstr(buf, INPUT1));
            assert(lstr_cat_cstr(buf, INPUT2));
            assert(strcmp(buf->content, INPUT1 INPUT2) == 0);
        });

        it("handles NULL gracefully", {
            struct lstr* buf = lbuf(1);
            defer(lstr_free(buf));
            assert(lstr_cat_cstr(NULL, NULL) == NULL);
            assert(lstr_cat_cstr(NULL, "test") == NULL);
            assert(lstr_cat_cstr(buf, NULL) == NULL);
        });
    });

    subdesc(lstr_cat_cstr, {
        it("concatenates multiple char into buf", {
            struct lstr* buf = lbuf(3);
            defer(lstr_free(buf));
            assert(lstr_cat_char(buf, 'a'));
            assert(lstr_cat_char(buf, 'b'));
            assert(lstr_cat_char(buf, 'c'));
            assert(strcmp(buf->content, "abc") == 0);
        });
    });
});

snow_main();
