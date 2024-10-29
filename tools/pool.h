#include <stddef.h>

struct pool_chunk;

struct pool {
        struct pool_chunk *chunk;
};

void pool_init(struct pool *pool);
void *pool_malloc(struct pool *pool, size_t sz);
void pool_free(struct pool *pool, void *p);
void pool_all_free(struct pool *pool);
size_t pool_size(const struct pool *pool);
