#include <assert.h>
#include <stdlib.h>

#include "defs.h"
#include "maputil.h"
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

void
prev_map(const struct node *n, map_t map)
{
        map_copy(map, n->map);

        /* undo */
        assert(map[n->loc] == _);
        assert(is_player(map[next_loc(n)]));
        assert((~n->flags & (MOVE_GET_BOMB | MOVE_PUSH)) != 0);
        move_object(map, n->loc, next_loc(n));
        if ((n->flags & MOVE_GET_BOMB) != 0) {
                map[next_loc(n)] = X;
        } else if ((n->flags & MOVE_PUSH) != 0) {
                assert(can_push(map[pushed_obj_loc(n)]));
                move_object(map, next_loc(n), pushed_obj_loc(n));
        }
}
