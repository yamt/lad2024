#if !defined(_LAD_SOLVER_NODE_H)
#define _LAD_SOLVER_NODE_H

#include <stdbool.h>

#include "list.h"
#include "rule.h"

struct node {
        map_t map;
        LIST_ENTRY(struct node) q;
        LIST_ENTRY(struct node) hashq;
        struct node *parent;
        unsigned int steps;

        /* move from the parent. note: this is redundant. */
        loc_t loc;
        enum diridx dir;
        unsigned int flags;
};

LIST_HEAD_NAMED(struct node, node_list);

void node_allocator_init(void);
struct node *alloc_node(void);
void free_node(struct node *node);
void free_all_nodes(void);
loc_t next_loc(const struct node *n);
loc_t pushed_obj_loc(const struct node *n);
bool is_trivial(const struct node *n, const map_t map);
void prev_map(const struct node *n, map_t map);

#endif /* !defined(_LAD_SOLVER_NODE_H) */
