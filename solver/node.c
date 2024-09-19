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
