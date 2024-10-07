#include <stdlib.h>

#include "node.h"
#include "pool.h"

struct pool nodepool;

void
node_allocator_init(void)
{
        pool_init(&nodepool, sizeof(struct node));
}

struct node *
alloc_node(void)
{
        struct node *n = pool_item_alloc(&nodepool);
        if (n == NULL) {
                exit(1);
        }
        return n;
}

void
free_node(struct node *n)
{
        pool_item_free(&nodepool, n);
}

void
free_all_nodes(void)
{
        pool_all_items_free(&nodepool);
}

loc_t
next_loc(const struct node *n)
{
        return n->loc + dirs[n->dir].loc_diff;
}

loc_t
pushed_obj_loc(const struct node *n)
{
        return n->loc + dirs[n->dir].loc_diff * 2;
}
