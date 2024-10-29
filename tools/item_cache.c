#include <assert.h>

#include "item_cache.h"
#include "pool.h"

struct pool_item_hdr {
        void *nextfree;
};

void
item_cache_init(struct item_cache *cache, struct pool *pool, size_t itemsize)
{
        cache->pool = pool;
        cache->itemsize = itemsize;
        cache->freelist = NULL;
}

void *
item_alloc(struct item_cache *cache)
{
        struct pool_item_hdr *h = cache->freelist;
        if (h != NULL) {
                cache->freelist = h->nextfree;
                return h;
        }
        return pool_malloc(cache->pool, cache->itemsize);
}

void
item_free(struct item_cache *cache, void *item)
{
        assert(item != NULL);
        struct pool_item_hdr *h = item;
        h->nextfree = cache->freelist;
        cache->freelist = h;
}

void
item_all_free(struct item_cache *cache)
{
        pool_all_free(cache->pool);
        cache->freelist = NULL;
}
