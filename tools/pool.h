#include <stddef.h>

struct pool {
        struct pool_item_hdr *freelist;
        struct pool_chunk *chunk;
        size_t itemsize;
};

void pool_init(struct pool *pool, size_t itemsize);
void *pool_item_alloc(struct pool *pool);
void pool_item_free(struct pool *pool, void *item);
void pool_all_items_free(struct pool *pool);
