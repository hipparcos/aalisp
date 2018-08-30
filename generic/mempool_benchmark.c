#include "mempool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BENCHMARK_IMPL
#include "../benchmark.h"

#ifndef RUNS
#define RUNS 100000
#endif

struct payload {
    size_t id;
    char large_object[40];
};

int main(void)
{
    size_t runs = RUNS;
    long long stt, end;
    size_t blockc = 512;
    struct payload payload = {0};

    {
    benchmark_display_banner("mempool", runs, "");
    stt = benchmark_get_time_ns();
    struct mp_pool* pool = mp_pool_alloc(blockc, sizeof(struct payload));
    uint64_t handle[blockc];
    struct payload* payload_ptr;
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            mp_alloc(pool, &handle[b]);
            mp_put(pool, handle[b], &payload);
        }
        assert(mp_pool_is_full(pool));
        for (size_t b = 0; b < blockc; b++) {
            assert(NULL != (payload_ptr = (struct payload*)mp_get(pool, handle[b])));
            assert(payload_ptr->id == r);
            mp_free(pool, handle[b]);
        }
    }
    mp_pool_free(&pool);
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs);
    }

    {
    benchmark_display_banner("malloc", runs, "");
    stt = benchmark_get_time_ns();
    struct payload* handle[blockc];
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            handle[b] = malloc(sizeof(struct payload));
            *handle[b] = payload;
        }
        payload.id = 0;
        for (size_t b = 0; b < blockc; b++) {
            payload = *handle[b];
            assert(payload.id == r);
            free(handle[b]);
        }
    }
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs);
    }

    return EXIT_SUCCESS;
}
