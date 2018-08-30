#ifndef H_MEMPOOL_
#define H_MEMPOOL_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** mp_cluster is a cluster of mp_pool.
 ** A cluster can grow or shrink. */
struct mp_cluster;

/** mp_pool is a generic fixed-size memory pool. */
struct mp_pool;

/** mp_pool_alloc allocates a pool for blockc blocks of size block_size. */
struct mp_pool* mp_pool_alloc(size_t blockc, size_t block_size);
/** mp_pool_free frees the whole pool. It set the pointer to NULL. */
void mp_pool_free(struct mp_pool**);
/** mp_pool_is_empty tells if the pool is empty (no allocated blocks). */
bool mp_pool_is_empty(struct mp_pool*);
/** mp_pool_is_full tells if the pool is full (all blocks are allocated). */
bool mp_pool_is_full(struct mp_pool*);
/** mp_alloc_from_pool allocates a new block from the pool. See mp_alloc. */
bool mp_alloc_from_pool(struct mp_pool*, uint64_t* handle);
/** mp_free_from_pool frees the block designated by handle. See mp_free. */
bool mp_free_from_pool(struct mp_pool*, uint64_t handle);
/** mp_get_from_pool returns a pointer to the content of the block designated by handle.
 ** See mp_get. */
const void* mp_get_from_pool(struct mp_pool*, uint64_t handle);
/** mp_put_to_pool set the memory of the block designated by handle. See mp_put. */
bool mp_put_to_pool(struct mp_pool*, uint64_t handle, const void* block);

/** mp_cluster_alloc allocates a new cluster of pool.
 ** pool must have been allocated beforehand.
 ** pool must not be freed, the cluster will do it for you. */
struct mp_cluster* mp_cluster_alloc(struct mp_pool* pool);
/** mp_cluster_free frees the whole cluster. It set the pointer to NULL. */
void mp_cluster_free(struct mp_cluster**);
/** mp_cluster_size returns the number of allocated pools. */
size_t mp_cluster_size(struct mp_cluster*);
/** mp_alloc_from_cluster allocates a new block from the cluster. See mp_alloc. */
bool mp_alloc_from_cluster(struct mp_cluster*, uint64_t* handle);
/** mp_free_from_cluster frees the block designated by handle. See mp_free. */
bool mp_free_from_cluster(struct mp_cluster*, uint64_t handle);
/** mp_get_from_cluster returns a pointer to the content of the block designated by handle.
 ** See mp_get. */
const void* mp_get_from_cluster(struct mp_cluster*, uint64_t handle);
/** mp_put_to_cluster set the memory of the block designated by handle. See mp_put. */
bool mp_put_to_cluster(struct mp_cluster*, uint64_t handle, const void* block);

/** mp_alloc allocates a block and returns its handle.
 ** Handles can safely be copied and used after mp_free was called. */
#define mp_alloc(pool, handle) _Generic((pool), \
        struct mp_pool*:    mp_alloc_from_pool, \
        struct mp_cluster*: mp_alloc_from_cluster)(pool, handle)
/** mp_free frees the block designated by handle. */
#define mp_free(pool, handle) _Generic((pool), \
        struct mp_pool*:    mp_free_from_pool, \
        struct mp_cluster*: mp_free_from_cluster)(pool, handle)
/** mp_get returns a pointer to the content of the block designated by handle.
 ** If the handle is not valid, mp_get returns NULL.
 ** A newly allocated block could not be get if put has not been called first.
 ** The returned pointer gives direct read-only access to memory. */
#define mp_get(pool, handle) _Generic((pool), \
        struct mp_pool*:    mp_get_from_pool, \
        struct mp_cluster*: mp_get_from_cluster)(pool, handle)
/** mp_put set the memory of the block designated by handle.
 ** If the handle is not valid, mp_put returns false.
 ** The content of block is copied. */
#define mp_put(pool, handle, payload) _Generic((pool), \
        struct mp_pool*:    mp_put_to_pool, \
        struct mp_cluster*: mp_put_to_cluster)(pool, handle, payload)

#endif
