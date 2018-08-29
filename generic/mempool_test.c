#include "mempool.h"

#include "../vendor/snow/snow/snow.h"

describe(mempool, {
    it("inits a mempool", {
        struct mp_pool* pool = mp_pool_alloc(2, sizeof(int));
        assert(mp_pool_is_empty(pool));
        assert(!mp_pool_is_full(pool));
        mp_pool_free(&pool);
        assert(pool == NULL);
    });

    it("puts 1 element in a mempool and retrieves it", {
        struct mp_pool* pool = mp_pool_alloc(2, sizeof(int));
        defer(mp_pool_free(&pool));
        uint64_t idx1;
        int int1 = -10;
        int *got = NULL;
        assert(mp_alloc(pool, &idx1));
        assert(mp_put(pool, idx1, &int1));
        assert(got = (int*)mp_get(pool, idx1));
        assert(*got == int1);
        assert(mp_free(pool, idx1));
    });

    it("fails to get from not allocated blocks", {
        struct mp_pool* pool = mp_pool_alloc(2, sizeof(int));
        defer(mp_pool_free(&pool));
        uint64_t idx1 = 1, idx2 = 2;
        assert(NULL == mp_get(pool, idx1));
        assert(NULL == mp_get(pool, idx2));
        assert(mp_alloc(pool, &idx1));
        assert(NULL != mp_get(pool, idx1));
        assert(mp_free(pool, idx1));
        assert(NULL == mp_get(pool, idx1));
        assert(NULL == mp_get(pool, idx2));
    });

    it("fails to put to not allocated blocks", {
        struct mp_pool* pool = mp_pool_alloc(2, sizeof(int));
        defer(mp_pool_free(&pool));
        uint64_t idx1 = 1, idx2 = 2;
        int i = -1;
        assert(!mp_put(pool, idx1, &i));
        assert(!mp_put(pool, idx2, &i));
        assert(mp_alloc(pool, &idx1));
        assert(mp_put(pool, idx1, &i));
        assert(mp_free(pool, idx1));
        assert(!mp_put(pool, idx1, &i));
        assert(!mp_put(pool, idx2, &i));
    });

    it("fills a mempool then remove all elements", {
        struct mp_pool* pool = mp_pool_alloc(2, sizeof(int));
        defer(mp_pool_free(&pool));
        assert(mp_pool_is_empty(pool));
        assert(!mp_pool_is_full(pool));
        uint64_t idx1, idx2, idx3;
        assert(mp_alloc(pool, &idx1));
        assert(mp_alloc(pool, &idx2));
        assert(!mp_alloc(pool, &idx3));
        assert(!mp_pool_is_empty(pool));
        assert(mp_pool_is_full(pool));
        assert(mp_free(pool, idx1));
        assert(mp_free(pool, idx2));
        assert(!mp_free(pool, idx3));
        assert(mp_pool_is_empty(pool));
        assert(!mp_pool_is_full(pool));
        assert(mp_alloc(pool, &idx3));
    });

    it("repeatedly allocates and frees an element", {
        struct mp_pool* pool = mp_pool_alloc(3, sizeof(int));
        defer(mp_pool_free(&pool));
        uint64_t idx1, idx2, idx3;
        int int1 = 10, int2 = 20, int3 = 30;
        assert(mp_alloc(pool, &idx1));
        assert(mp_put(pool, idx1, &int1));
        assert(mp_alloc(pool, &idx2));
        assert(mp_put(pool, idx2, &int2));
        assert(mp_alloc(pool, &idx3));
        assert(mp_put(pool, idx3, &int3));
        int *got = NULL;
        assert(got = (int*)mp_get(pool, idx1));
        assert(*got == int1);
        assert(got = (int*)mp_get(pool, idx2));
        assert(*got == int2);
        assert(got = (int*)mp_get(pool, idx3));
        assert(*got == int3);
        int2 = 42;
        assert(mp_free(pool, idx2));
        assert(NULL == mp_get(pool, idx2));
        assert(mp_alloc(pool, &idx2));
        assert(mp_put(pool, idx2, &int2));
        assert(got = (int*)mp_get(pool, idx1));
        assert(*got == int1);
        assert(got = (int*)mp_get(pool, idx2));
        assert(*got == int2);
        assert(got = (int*)mp_get(pool, idx3));
        assert(*got == int3);
    });

    it("fills then frees all elements from the pool 10 times", {
        size_t blockc = 10;
        struct mp_pool* pool = mp_pool_alloc(blockc, sizeof(int));
        defer(mp_pool_free(&pool));
        uint64_t handle[10], h = 0;
        for (size_t i = 1; i < 10; i++) {
            h = 0;
            while (!mp_pool_is_full(pool)) {
                int value = i * h;
                assert(mp_alloc(pool, &handle[h]));
                assert(mp_put(pool, handle[h], &value));
                h++;
            }
            assert(h == blockc);
            h = 0;
            while (!mp_pool_is_empty(pool)) {
                int value = i * h;
                assert(value == *(int*)mp_get(pool, handle[h]));
                assert(mp_free(pool, handle[h]));
                h++;
            }
        }
    });
});

snow_main();
