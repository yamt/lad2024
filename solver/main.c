#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "dump.h"
#include "evaluater.h"
#include "list.h"
#include "loader.h"
#include "node.h"
#include "solver.h"

int
main(int argc, char **argv)
{
        if (argc != 2) {
                exit(2);
        }
        int stage_number = atoi(argv[1]);
        if (stage_number <= 0) {
                exit(2);
        }

        struct node *n = alloc_node();
        struct map_info info;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, n->map, &info);
        if (info.w > width || info.h > height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", width, height);
                exit(1);
        }
        dump_map(n->map);

        struct node_list solution;
        unsigned int result = solve(n, 10000000, &solution);
        if (result == SOLVE_SOLVED) {
                struct evaluation ev;
                evaluate(&solution, &ev);
                printf("score %u\n", ev.score);
        }
        solve_cleanup();
}
