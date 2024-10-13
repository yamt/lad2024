#include <stdlib.h>

#include "defs.h"
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

bool
is_trivial(const struct node *n, const map_t beam)
{
        if ((n->flags & (MOVE_PUSH | MOVE_GET_BOMB)) != 0) {
                return false;
        }
        loc_t nloc = next_loc(n);
        bool is_robot = n->map[nloc] == A;
        if ((beam[nloc] != 0) != is_robot) {
                return false;
        }
        return true;
}
