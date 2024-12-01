#if !defined(_LAD_SOLVER_NODE_H)
#define _LAD_SOLVER_NODE_H

#include <stdbool.h>

#include "list.h"
#include "slist.h"
#include "rule.h"

struct node {
        SLIST_ENTRY(struct node) q;
#if !defined(SMALL_NODE)
        LIST_ENTRY(struct node) hashq;
#else
        SLIST_ENTRY(struct node) hashq;
#endif
        struct node *parent;
        unsigned int steps;
#if defined(NODE_KEEP_HASH)
        uint32_t hash;
#endif

        /* move from the parent. note: this is redundant. */
        loc_t loc;
        enum diridx dir;
        move_flags_t flags;

#if !defined(SMALL_NODE)
        map_t map;
#endif
};

LIST_HEAD_NAMED(struct node, node_list);

void node_allocator_init(void);
struct node *alloc_node(void);
void free_node(struct node *node);
void free_all_nodes(void);
loc_t next_loc(const struct node *n);
loc_t pushed_obj_loc(const struct node *n);
bool is_trivial(const struct node *n, const map_t map, const map_t beam_map);
void node_apply(const struct node *n, map_t map);
void node_undo(const struct node *n, map_t map);
void prev_map(const struct node *n, const map_t node_map, map_t map);
void node_expand_map(const struct node *n, const map_t root, map_t map);

#endif /* !defined(_LAD_SOLVER_NODE_H) */
