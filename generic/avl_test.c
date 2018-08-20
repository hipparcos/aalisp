#include "avl.h"

#include <stdio.h>
#include <string.h>

#include "../vendor/snow/snow/snow.h"

struct payload {
    int key;
    char as_string[12];
};

static struct payload* payload_alloc(int key) {
    struct payload* pl = calloc(1, sizeof(struct payload));
    pl->key = key;
    snprintf(pl->as_string, sizeof(pl->as_string), "%2d", key);
    return pl;
}

static void* payload_copy(const void* srcv) {
    struct payload* src = (struct payload*)srcv;
    struct payload* dest = payload_alloc(src->key);
    return (void*)dest;
}

static void payload_free(void* datav) {
    struct payload* data = (struct payload*)datav;
    free(data);
}

static int payload_cmp(const void* leftv, const void* rightv) {
    struct payload* left = (struct payload*)leftv;
    struct payload* right = (struct payload*)rightv;
    int diff = left->key - right->key;
    return (diff > 0) - (diff < 0);
}

size_t payload_printer(FILE* out, const void* datav) {
    struct payload* data = (struct payload*)datav;
    fputs(data->as_string, out);
    return strlen(data->as_string);
}

#define LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))


describe(avl, {
    it("inits an AVL and frees it", {
        struct payload* pl = payload_alloc(10);
        struct avl_node* tree = avl_alloc(pl);
        defer(avl_free(tree, payload_free));
        assert(avl_size(tree) == 1);
    });

#define init_avl(tree) \
    do { \
        tree = avl_alloc(NULL); \
        int list[] = {3, 72, 54, 42, 90, 1, 23, 99, 18, 65}; \
        defer(if (tree) avl_free(tree, payload_free)); \
        for (size_t i = 0; i < LENGTH(list); i++) { \
            bool insertion = false; \
            struct payload* pl = payload_alloc(list[i]); \
            struct avl_node* node = avl_alloc(pl); \
            tree = avl_insert(tree, node, payload_cmp, payload_free, &insertion); \
        } \
        assert(avl_size(tree) == LENGTH(list)); \
        assert(avl_is_balanced(tree)); \
    } while (0);

    it("puts element into an AVL then displays it", {
        struct avl_node* tree = NULL;
        init_avl(tree);
        fputc('\n', stdout);
        avl_print_to(tree, stdout, payload_printer);
    });

    it("lookups for existing element into an AVL", {
        struct avl_node* tree = NULL;
        init_avl(tree);
        struct payload lku = {.key= 42};
        const struct payload* got = NULL;
        got = avl_lookup(tree, payload_cmp, &lku);
        assert(got && got->key == 42);
    });

    it("lookups for non existing element into an AVL", {
        struct avl_node* tree = NULL;
        init_avl(tree);
        struct payload lku = {.key= 0};
        const struct payload* got = NULL;
        got = avl_lookup(tree, payload_cmp, &lku);
        assert(!got);
    });

    it("copies an AVL, frees original, lookup in copy", {
        struct avl_node* src = NULL;
        init_avl(src);
        size_t size_src = avl_size(src);
        struct avl_node* dest = avl_duplicate(src, payload_copy);
        defer(avl_free(dest, payload_free));
        assert(avl_size(dest) == size_src);
        avl_free(src, payload_free);
        src = NULL;
        assert(avl_size(dest) == size_src);
        /* Lookup. */
        struct payload lku = {.key= 42};
        const struct payload* got = NULL;
        got = avl_lookup(dest, payload_cmp, &lku);
        assert(got && got->key == 42);
    });
});

snow_main();
