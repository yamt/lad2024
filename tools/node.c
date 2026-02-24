#include <assert.h>
#include <stdlib.h>

#include "defs.h"
#include "item_cache.h"
#include "maputil.h"
#include "node.h"
#include "pool.h"

struct pool nodepool;
struct item_cache nodecache;

void
node_allocator_init(void)
{
        pool_init(&nodepool);
        item_cache_init(&nodecache, &nodepool, sizeof(struct node));
}

struct node *
alloc_node(void)
{
        struct node *n = item_alloc(&nodecache);
        if (n == NULL) {
                exit(1);
        }
        return n;
}

void
free_node(struct node *n)
{
        item_free(&nodecache, n);
}

void
free_all_nodes(void)
{
        item_all_free(&nodecache);
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
is_trivial(const struct node *n, const map_t map, const map_t beam)
{
        if (n->steps == 0 || (n->flags & (MOVE_PUSH | MOVE_GET_BOMB)) != 0) {
                return false;
        }
        loc_t nloc = next_loc(n);
        bool is_robot = map[nloc] == A;
        if ((beam[nloc] != 0) != is_robot) {
                return false;
        }
        return true;
}

void
node_undo(const struct node *n, map_t map)
{
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

void
prev_map(const struct node *n, const map_t node_map, map_t map)
{
        map_copy(map, node_map);
        node_undo(n, map);
}

void
node_apply(const struct node *n, map_t map)
{
        assert(n->steps > 0);
        if ((n->flags & MOVE_PUSH) != 0) {
                move_object(map, pushed_obj_loc(n), next_loc(n));
        }
        move_object(map, next_loc(n), n->loc);
}

static void
node_recursively_apply(const struct node *n, map_t map)
{
        if (n->steps > 1) {
                node_recursively_apply(n->parent, map);
        }
        if (n->steps > 0) {
                node_apply(n, map);
        }
}

void
node_expand_map(const struct node *n, const map_t root, map_t map)
{
        map_copy(map, root);
        node_recursively_apply(n, map);
}
