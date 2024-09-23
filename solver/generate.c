#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "evaluater.h"
#include "list.h"
#include "loader.h"
#include "node.h"
#include "solver.h"

void
generate(map_t map)
{
        int h = 8;
        int w = 8;

        memset(map, 0, genloc(map_width, map_height));
        int x;
        int y;
        for (y = 0; y < h; y++) {
                for (x = 0; x < w; x++) {
                        map[genloc(x, y)] = W;
                }
        }
}

int
main(int argc, char **argv)
{
        struct node *n = alloc_node();
        generate(n->map);
        dump_map(n->map);
}
