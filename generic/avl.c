#include "avl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct avl_node {
    struct avl_node* left;
    struct avl_node* right;
    void* payload;
};

/** avl_nil is the empty node. */
static struct avl_node avl_nil = {0};

struct avl_node* avl_alloc(void* payload) {
    if (!payload) {
        return NULL;
    }
    struct avl_node* node = calloc(1, sizeof(struct avl_node));
    node->left = &avl_nil;
    node->right = &avl_nil;
    node->payload = payload;
    return node;
};

bool avl_is_nil(const struct avl_node* tree) {
    return tree == NULL || tree == &avl_nil;
}

void avl_free(struct avl_node* tree, avl_pl_destructor destructor) {
    if (avl_is_nil(tree)) {
        return;
    }
    avl_free(tree->left, destructor);
    avl_free(tree->right, destructor);
    destructor(tree->payload);
    free(tree);
}

struct avl_node* avl_duplicate(const struct avl_node* src, avl_pl_duplicator copy) {
    if (avl_is_nil(src)) {
        return &avl_nil;
    }
    struct avl_node* dest = avl_alloc(src->payload);
    dest->left = avl_duplicate(src->left, copy);
    dest->right = avl_duplicate(src->right, copy);
    dest->payload = copy(src->payload);
    return dest;
}

static struct avl_node* avl_rotate_left(struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return &avl_nil;
    }
    struct avl_node* new_left = tree;
    struct avl_node* new_left_right = tree->right->left;
    tree = tree->right;
    tree->left = new_left;
    new_left->right = new_left_right;
    return tree;
}

static struct avl_node* avl_rotate_right(struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return &avl_nil;
    }
    struct avl_node* new_right = tree;
    struct avl_node* new_right_left = tree->left->right;
    tree = tree->left;
    tree->right = new_right;
    new_right->left = new_right_left;
    return tree;
}

#define max(a,b) ((a > b) ? a : b)
static int avl_height(struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return 0;
    }
    return 1 + max(avl_height(tree->left), avl_height(tree->right));
}

static int avl_balance_factor(const struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return 0;
    }
    return avl_height(tree->right) - avl_height(tree->left);
}

/** avl_balance balances tree and returns the new root. */
static struct avl_node* avl_balance(struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return &avl_nil;
    }
    int balance_factor = avl_balance_factor(tree);
    if (balance_factor >  1) {
        if (avl_balance_factor(tree->right) < -1) {
            tree->right = avl_rotate_right(tree->right);
        }
        tree = avl_rotate_left(tree);
    }
    if (balance_factor < -1) {
        if (avl_balance_factor(tree->right) >  1) {
            tree->left = avl_rotate_left(tree->left);
        }
        tree = avl_rotate_right(tree);
    }
    return tree;
}

/** avl_insert inserts node in tree then returns the new root of the tree. */
struct avl_node* avl_insert(struct avl_node* tree, struct avl_node* node,
        avl_pl_comparator comparator, avl_pl_destructor destructor,
        bool* insertion) {
    if (avl_is_nil(node)) {
        return tree;
    }
    if (avl_is_nil(tree)) {
        *insertion = true;
        return node;
    }
    *insertion = false;
    int s = comparator(node->payload, tree->payload);
    /* Override. */
    if (s == 0) {
        node->left = tree->left;
        node->right = tree->right;
        tree->left = &avl_nil;
        tree->right = &avl_nil;
        avl_free(tree, destructor);
        return node;
    }
    /* Find  the right place. */
    if (s < 0) {
        tree->left = avl_insert(tree->left, node, comparator, destructor, insertion);
    }
    if (s > 0) {
        tree->right = avl_insert(tree->right, node, comparator, destructor, insertion);
    }
    /* Balance. */
    if (*insertion) {
        tree = avl_balance(tree);
    }
    return tree;
}

/** avl_lookup returns the node identified by key or NULL. */
const void* avl_lookup(struct avl_node* tree,
        avl_pl_comparator comparator, const void* key) {
    if (avl_is_nil(tree)) {
        return NULL;
    }
    int s = comparator(key, tree->payload);
    if (s < 0) {
        return avl_lookup(tree->left, comparator, key);
    }
    if (s > 0) {
        return avl_lookup(tree->right, comparator, key);
    }
    if (s == 0) {
        return tree->payload;
    }
    return NULL;
}

size_t avl_size(const struct avl_node* tree) {
    if (avl_is_nil(tree)) {
        return 0;
    }
    return 1 + avl_size(tree->left) + avl_size(tree->right);
}

#define abs(x) ((x < 0) ? -x : x)
bool avl_is_balanced(const struct avl_node* tree) {
    return abs(avl_balance_factor(tree)) < 2 &&
           abs(avl_balance_factor(tree->left)) < 2 &&
           abs(avl_balance_factor(tree->right)) < 2;
}

/** avl_keys_recurse fills list by doing a depth first traversal of tree. */
static const char** avl_keys_recurse(const struct avl_node* tree,
        avl_pl_serializer serializer, const char** list) {
    if (avl_is_nil(tree)) {
        return list;
    }
    list = avl_keys_recurse(tree->left, serializer, list);
    *(list++) = serializer(tree->payload);
    list = avl_keys_recurse(tree->right, serializer, list);
    return list;
}

const char** avl_keys(const struct avl_node* tree,
        avl_pl_serializer serializer, size_t* len) {
    if (avl_is_nil(tree)) {
        return NULL;
    }
    *len = avl_size(tree);
    if (*len == 0) {
        return NULL;
    }
    const char** list = calloc(*len, sizeof(char*));
    avl_keys_recurse(tree, serializer, list);
    return list;
}

static void avl_print_to_indent(const struct avl_node* tree, FILE* out,
        avl_pl_printer printer, size_t indent) {
    if (!tree->payload) {
        fputs(" > nil", out);
        return;
    }
    fputs(" > ", out);
    indent += 3;
    indent += printer(out, tree->payload);
    avl_print_to_indent(tree->left, out, printer, indent);
    fputc('\n', out);
    /* Indent. */
    size_t spaces = indent;
    while (spaces--) {
        fputc(' ', out);
    }
    avl_print_to_indent(tree->right, out, printer, indent);
}

void avl_print_to(const struct avl_node* tree, FILE* out,
        avl_pl_printer printer) {
    avl_print_to_indent(tree, out, printer, 0);
    fputc('\n', out);
}
