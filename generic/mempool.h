#ifndef H_MEMPOOL_
#define H_MEMPOOL_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** mp_pool is a generic memory pool. */
struct mp_pool;

/** mp_pool_alloc allocates a pool for blockc blocks of size block_size. */
struct mp_pool* mp_pool_alloc(size_t blockc, size_t block_size);
/** mp_pool_free frees the whole pool. It set the pointer to NULL. */
void mp_pool_free(struct mp_pool**);
/** mp_pool_is_empty tells if the pool is empty (no allocated blocks). */
bool mp_pool_is_empty(struct mp_pool*);
/** mp_pool_is_full tells if the pool is full (all blocks are allocated). */
bool mp_pool_is_full(struct mp_pool*);

/** mp_alloc allocates a block and returns its handle.
 ** Handles can safely be copied and used after mp_free was called. */
bool mp_alloc(struct mp_pool*, uint64_t* handle);
/** mp_free frees the block designated by handle. */
bool mp_free(struct mp_pool*, uint64_t handle);
/** mp_get returns a pointer to the content of the block designated by handle.
 ** If the handle is not valid, mp_get returns NULL.
 ** A newly allocated block could not be get if put has not been called first.
 ** The returned pointer gives direct read-only access to memory. */
const void* mp_get(struct mp_pool*, uint64_t handle);
/** mp_put set the memory of the block at handle.
 ** If the handle is not valid, mp_put returns false.
 ** The content of block is copied. */
bool mp_put(struct mp_pool*, uint64_t handle, const void* block);

#endif
