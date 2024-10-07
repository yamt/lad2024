#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pool.h"

#define POOL_CHUNK_SIZE (128 * 1024 * 1024)

struct pool_item_hdr {
        void *nextfree;
};

struct pool_chunk {
        struct pool_chunk *next;
        uint8_t data[];
};

static size_t
pool_items_per_chunk(const struct pool *pool)
{
        size_t chunk_size = POOL_CHUNK_SIZE;
        size_t payload_size = chunk_size - sizeof(struct pool_chunk);
        size_t nitems = payload_size / pool->itemsize;
        return nitems;
}

static void
pool_chunk_alloc(struct pool *pool)
{
        size_t nitems = pool_items_per_chunk(pool);
        assert(nitems > 0);
        struct pool_chunk *chunk = malloc(POOL_CHUNK_SIZE);
        if (chunk == NULL) {
                return;
        }
        size_t i;
        for (i = 0; i < nitems; i++) {
                void *item = &chunk->data[i * pool->itemsize];
                pool_item_free(pool, item);
        }
        chunk->next = pool->chunk;
        pool->chunk = chunk;
        return;
}

void
pool_init(struct pool *pool, size_t itemsize)
{
        assert(itemsize >= sizeof(struct pool_item_hdr));
        pool->freelist = NULL;
        pool->chunk = NULL;
        pool->itemsize = itemsize;
        assert(pool_items_per_chunk(pool) > 0);
}

void
pool_all_items_free(struct pool *pool)
{
        struct pool_chunk *next = pool->chunk;
        struct pool_chunk *chunk;
        while ((chunk = next) != NULL) {
                next = chunk->next;
                free(chunk);
        }
        pool->freelist = NULL;
        pool->chunk = NULL;
}

void *
pool_item_alloc(struct pool *pool)
{
        struct pool_item_hdr *h = pool->freelist;
        if (h == NULL) {
                pool_chunk_alloc(pool);
                h = pool->freelist;
                if (h == NULL) {
                        return NULL;
                }
        }
        assert(h != NULL);
        pool->freelist = h->nextfree;
        return h;
}

void
pool_item_free(struct pool *pool, void *item)
{
        assert(item != NULL);
        struct pool_item_hdr *h = item;
        h->nextfree = pool->freelist;
        pool->freelist = h;
}
