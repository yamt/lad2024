#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pool.h"

#define POOL_CHUNK_SIZE (128 * 1024 * 1024)
#define POOL_MALLOC_ALIGN 16

struct pool_chunk {
        struct pool_chunk *nextchunk;
        uint8_t *nextalloc;
        uint8_t data[];
};

static void *
align_up(void *p, size_t sz)
{
        return (void *)(((uintptr_t)p + sz - 1) & -sz);
}

static void
pool_chunk_alloc(struct pool *pool)
{
        struct pool_chunk *chunk = malloc(POOL_CHUNK_SIZE);
        if (chunk == NULL) {
                return;
        }
        chunk->nextalloc = align_up(chunk->data, POOL_MALLOC_ALIGN);

        chunk->nextchunk = pool->chunk;
        pool->chunk = chunk;
        return;
}

static void *
pool_chunk_malloc(struct pool_chunk *chunk, size_t sz)
{
        const uint8_t *end = (const uint8_t *)chunk + POOL_CHUNK_SIZE;
        if (chunk != NULL && chunk->nextalloc + sz <= end) {
                void *p = chunk->nextalloc;
                chunk->nextalloc =
                        align_up(chunk->nextalloc + sz, POOL_MALLOC_ALIGN);
                return p;
        }
        return NULL;
}

void
pool_init(struct pool *pool)
{
        pool->chunk = NULL;
}

void *
pool_malloc(struct pool *pool, size_t sz)
{
        void *p;
        p = pool_chunk_malloc(pool->chunk, sz);
        if (p == NULL) {
                pool_chunk_alloc(pool);
                p = pool_chunk_malloc(pool->chunk, sz);
        }
        return p;
}

void
pool_free(struct pool *pool, void *p)
{
        /* nothing for now */
}

void
pool_all_free(struct pool *pool)
{
        struct pool_chunk *next = pool->chunk;
        struct pool_chunk *chunk;
        while ((chunk = next) != NULL) {
                next = chunk->nextchunk;
                free(chunk);
        }
        pool->chunk = NULL;
}

size_t
pool_size(const struct pool *pool)
{
        size_t sz = 0;
        struct pool_chunk *chunk;
        for (chunk = pool->chunk; chunk != NULL; chunk = chunk->nextchunk) {
                sz += POOL_CHUNK_SIZE;
        }
        return sz;
}
