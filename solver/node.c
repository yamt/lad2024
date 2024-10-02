#include <stdlib.h>

#include "node.h"

struct node *
alloc_node(void)
{
        struct node *n = malloc(sizeof(*n));
        if (n == NULL) {
                exit(1);
        }
        return n;
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
