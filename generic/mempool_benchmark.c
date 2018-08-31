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

    /* malloc/free */
    {
    benchmark_display_banner("malloc", runs * blockc, "base case");
    stt = benchmark_get_time_ns();
    struct payload* handles[blockc];
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            handles[b] = malloc(sizeof(struct payload));
            *handles[b] = payload;
        }
        payload.id = 0;
        for (size_t b = 0; b < blockc; b++) {
            payload = *handles[b];
            assert(payload.id == r);
            free(handles[b]);
        }
    }
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs * blockc);
    }

    /* A single pool. */
    {
    benchmark_display_banner("mempool", runs * blockc, "pool overhead");
    stt = benchmark_get_time_ns();
    struct mp_pool* pool = mp_pool_alloc(blockc, sizeof(struct payload));
    uint64_t handles[blockc];
    struct payload* payload_ptr;
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            mp_alloc(pool, &handles[b]);
            mp_put(pool, handles[b], &payload);
        }
        assert(mp_pool_is_full(pool));
        for (size_t b = 0; b < blockc; b++) {
            assert(NULL != (payload_ptr = (struct payload*)mp_get(pool, handles[b])));
            assert(payload_ptr->id == r);
            mp_free(pool, handles[b]);
        }
    }
    mp_pool_free(&pool);
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs * blockc);
    }

    /* A cluster of a single pool. */
    {
    benchmark_display_banner("mempool/cluster (1 pool)", runs * blockc, "cluster overhead");
    stt = benchmark_get_time_ns();
    struct mp_cluster* cluster =
        mp_cluster_alloc(mp_pool_alloc(blockc+1, sizeof(struct payload)));
    uint64_t handles[blockc];
    struct payload* payload_ptr;
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            mp_alloc(cluster, &handles[b]);
            mp_put(cluster, handles[b], &payload);
        }
        for (size_t b = 0; b < blockc; b++) {
            assert(NULL != (payload_ptr = (struct payload*)mp_get(cluster, handles[b])));
            assert(payload_ptr->id == r);
            mp_free(cluster, handles[b]);
        }
    }
    mp_cluster_free(&cluster);
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs * blockc);
    }

    /* A cluster of 3 pools. */
    {
    benchmark_display_banner("mempool/cluster (3 pools)", runs * blockc,
            "cluster growing/shrinking overhead");
    stt = benchmark_get_time_ns();
    struct mp_cluster* cluster =
        mp_cluster_alloc(mp_pool_alloc(blockc/3+1, sizeof(struct payload)));
    uint64_t handles[blockc];
    struct payload* payload_ptr;
    for (size_t r = 1; r <= runs; r++) {
        payload.id = r;
        for (size_t b = 0; b < blockc; b++) {
            mp_alloc(cluster, &handles[b]);
            mp_put(cluster, handles[b], &payload);
        }
        for (size_t b = 0; b < blockc; b++) {
            assert(NULL != (payload_ptr = (struct payload*)mp_get(cluster, handles[b])));
            assert(payload_ptr->id == r);
            mp_free(cluster, handles[b]);
        }
    }
    mp_cluster_free(&cluster);
    end = benchmark_get_time_ns();
    benchmark_display_results(stt, end, runs * blockc);
    }

    return EXIT_SUCCESS;
}
