/* fixed-sized object cache */

#include <stddef.h>

struct pool;

struct item_cache {
        struct pool *pool;
        struct pool_item_hdr *freelist;
        size_t itemsize;
};

void item_cache_init(struct item_cache *cache, struct pool *pool,
                     size_t itemsize);
void *item_alloc(struct item_cache *cache);
void item_free(struct item_cache *cache, void *item);
void item_all_free(struct item_cache *cache);
