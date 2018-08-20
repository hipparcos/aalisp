#ifndef H_AVL_GENERIC_
#define H_AVL_GENERIC_

#include <stdbool.h>
#include <stdlib.h>

/** avl_node is an AVL tree. */
struct avl_node;

/** avl_pl_destructor destroys a payload. */
typedef void (*avl_pl_destructor)(void*);
/** avl_pl_duplicator duplictes a payload. */
typedef void* (*avl_pl_duplicator)(const void*);
/** avl_pl_comparator compares two payload.
 ** It returns {-1,0,1} if left is {less than, equal to, greater than} right. */
typedef int (*avl_pl_comparator)(const void*, const void*);
/** avl_pl_serializer returns a string which describes a payload. */
typedef const char* (*avl_pl_serializer)(const void*);

/** avl_alloc returns a new AVL containing payload. */
struct avl_node* avl_alloc(void* payload);
/** avl_is_nil tells if tree is empty. */
bool avl_is_nil(const struct avl_node* tree);
/** avl_free recursively frees tree. */
void avl_free(struct avl_node* tree, avl_pl_destructor destructor);
/** avl_duplicate copies src. */
struct avl_node* avl_duplicate(const struct avl_node* src, avl_pl_duplicator copy);
/** avl_insert inserts node in tree then returns the new root of the tree. */
struct avl_node* avl_insert(struct avl_node* tree, struct avl_node* node,
        avl_pl_comparator comparator, avl_pl_destructor destructor,
        bool* insertion);
/** avl_lookup returns the payload identified by key or NULL. */
const void* avl_lookup(struct avl_node* tree,
        avl_pl_comparator comparator, const void* key);
/** avl_size returns the number of node in tree. */
size_t avl_size(const struct avl_node* tree);
/** avl_keys returns a list of all keys contained in AVL. */
const char** avl_keys(const struct avl_node* tree,
        avl_pl_serializer serializer, size_t* len);

#endif
