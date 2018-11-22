#include "lparser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "lerr.h"
#include "llexer.h"

#include "vendor/snow/snow/snow.h"

struct ast_list {
    size_t  id;
    size_t* children;
    enum last_type type;
    char* content;
    struct last* node;
};

/** ast_builder builds an ast from the given list.
 ** list must end with the LAST_PROG node and then an element with id=0. */
struct last* ast_builder(struct ast_list* list) {
    if (!list) {
        return NULL;
    }
    struct last *root = NULL;
    struct ast_list* curr = list;
    /* Create all ast nodes. */
    do {
        /* Create node. */
        struct last* node = calloc(1, sizeof(struct last));
        node->type = curr->type;
        size_t len = strlen(curr->content);
        node->content = malloc(len+1);
        strncpy(node->content, curr->content, len+1);
        /* Save node into current list element. */
        curr->node = node;
        /* Set new root. */
        root = node;
    } while (++curr && curr->id);
    /* Attach children. */
    curr = list;
    do {
        for (size_t* c = curr->children; c && *c; c++) {
            /* Look for existing child */
            struct ast_list* child = list;
            do {
                if (child->id == *c) {
                    break;
                }
            } while (++child && child->id);
            /* Append child to node. */
            struct last* node = curr->node;
            node->childrenc++;
            node->children = realloc(node->children, node->childrenc * sizeof(struct last*));
            node->children[node->childrenc-1] = child->node;
        }
    } while (++curr && curr->id);
    return root;
}

#define test_pass(msg, input, expected) \
    it("pass for " msg, { \
        struct last* expec = ast_builder(expected); \
        defer(last_free(expec)); \
        struct lerr* err = NULL; \
        defer(lerr_free(err)); \
        struct ltok* tokens = NULL; \
        defer(ltok_free(tokens)); \
        assert(tokens = llex(input, &err)); \
        struct last* got = NULL; \
        defer(last_free(got)); \
        assert(got = lparse(tokens, &err)); \
        if (!last_are_all_equal(got, expec)) { \
            puts("\ngot:"); \
            last_print_all(got); \
            puts("\nexpected:"); \
            last_print_all(expec); \
            fail("got != expected"); \
        } \
    })

#define test_fail(msg, input) \
    it("fail for " msg, { \
        struct ltok* tokens = NULL; \
        defer(ltok_free(tokens)); \
        struct lerr* err = NULL; \
        defer(lerr_free(err)); \
        assert(tokens = llex(input, &err)); \
        struct last* got = NULL; \
        defer(last_free(got)); \
        assert(!(got = lparse(tokens, &err)) || err != NULL); \
    })

describe(lparse, {

    test_pass("s-expression",
            "(+ 1 2)",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 3, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 4, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 3, 0}[0])},
                {.id= 5, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){4, 0}[0])},
                {.id= 6, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){5, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("s-expression",
            "(+ 1 +)",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 3, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 4, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 3, 0}[0])},
                {.id= 5, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){4, 0}[0])},
                {.id= 6, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){5, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("s-expression",
            "((+) 1 2)",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 2, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 0}[0])},
                {.id= 3, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){2, 0}[0])},
                {.id= 4, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 5, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 6, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){3, 4, 5, 0}[0])},
                {.id= 7, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){6, 0}[0])},
                {.id= 8, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){7, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("q-expression",
            "(head {1 2 3})",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "head", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 3, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 4, .type= LAST_NUM, .content= "3", .children= NULL},
                {.id= 5, .type= LAST_QEXPR, .content= "",
                    .children= &((size_t[]){2, 3, 4, 0}[0])},
                {.id= 6, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 5, 0}[0])},
                {.id= 7, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){6, 0}[0])},
                {.id= 8, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){7, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("doubles",
            "(+ 1.0 2.0)",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 2, .type= LAST_DBL, .content= "1.0", .children= NULL},
                {.id= 3, .type= LAST_DBL, .content= "2.0", .children= NULL},
                {.id= 4, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 3, 0}[0])},
                {.id= 5, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){4, 0}[0])},
                {.id= 6, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){5, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("strings",
            "(concat \"test\" \"esc\\\"aped\")",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "concat", .children= NULL},
                {.id= 2, .type= LAST_STR, .content= "test", .children= NULL},
                {.id= 3, .type= LAST_STR, .content= "esc\"aped", .children= NULL},
                {.id= 4, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 3, 0}[0])},
                {.id= 5, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){4, 0}[0])},
                {.id= 6, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){5, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("UTF-8 strings",
            "(concat \"عربى\" \"भारतीय\")",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "concat", .children= NULL},
                {.id= 2, .type= LAST_STR, .content= "عربى", .children= NULL},
                {.id= 3, .type= LAST_STR, .content= "भारतीय", .children= NULL},
                {.id= 4, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 3, 0}[0])},
                {.id= 5, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){4, 0}[0])},
                {.id= 6, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){5, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("nested s-expressions",
            "(+ 1 (! 2))",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "!", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 3, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 0}[0])},
                {.id= 4, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){3, 0}[0])},
                {.id= 5, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 6, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 7, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){5, 6, 4, 0}[0])},
                {.id= 8, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){7, 0}[0])},
                {.id= 9, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){8, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("nested s-expressions with $",
            "(+ 1 $ ! 2)",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "!", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 3, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 0}[0])},
                {.id= 4, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){3, 0}[0])},
                {.id= 5, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 6, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 7, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){5, 6, 4, 0}[0])},
                {.id= 8, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){7, 0}[0])},
                {.id= 9, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){8, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_pass("s-expressions with $ inside a q-expression",
            "(head {+ 1 $ ! 2})",
            &((struct ast_list[]){
                {.id= 1, .type= LAST_SYM, .content= "!", .children= NULL},
                {.id= 2, .type= LAST_NUM, .content= "2", .children= NULL},
                {.id= 3, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){1, 2, 0}[0])},
                {.id= 4, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){3, 0}[0])},
                {.id= 5, .type= LAST_SYM, .content= "+", .children= NULL},
                {.id= 6, .type= LAST_NUM, .content= "1", .children= NULL},
                {.id= 7, .type= LAST_QEXPR, .content= "",
                    .children= &((size_t[]){5, 6, 4, 0}[0])},
                {.id= 8, .type= LAST_SYM, .content= "head", .children= NULL},
                {.id= 9, .type= LAST_EXPR, .content= "",
                    .children= &((size_t[]){8, 7, 0}[0])},
                {.id= 10, .type= LAST_SEXPR, .content= "",
                    .children= &((size_t[]){9, 0}[0])},
                {.id= 11, .type= LAST_PROG, .content= "",
                    .children= &((size_t[]){10, 0}[0])},
                {.id= 0, .type= 0, .content= 0, .children= NULL}
            })[0]
        );

    test_fail("NULL", NULL);
    test_fail("0-length input", "");
    test_fail("s-expr which does not begin with a symbol", "(1 + 2)");
    test_fail("s-expr which does not end with a `)`", "(! 1");
    test_fail("s-expr that starts right before the end of a line", "(");
    test_fail("q-expr which does not end with a `}`", "(head {1 2 3)");

});

snow_main();
