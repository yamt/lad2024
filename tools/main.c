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

unsigned int
load_and_evaluate_stage(unsigned int stage_number, struct evaluation *ev)
{
        struct node *n = alloc_node();
        struct map_info info;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, n->map, &info);
        if (info.w > map_width || info.h > map_height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        dump_map(n->map);

        struct solution solution;
        unsigned int result = solve(n, &solver_default_param, true, &solution);
        if (result == SOLVE_SOLVED) {
                evaluate(n, &solution.moves, ev);
        }
        solve_cleanup();
        return result;
}

int
main(int argc, char **argv)
{
        if (argc != 2) {
                exit(2);
        }
        int stage_number = atoi(argv[1]);
        if (stage_number < 0) {
                exit(2);
        }

        node_allocator_init();
        if (stage_number > 0) {
                struct evaluation ev;
                load_and_evaluate_stage(stage_number, &ev);
        } else {
                struct result {
                        unsigned int solve_flags;
                        struct evaluation ev;
                } *results;
                results = calloc(sizeof(results), nstages);
                if (results == NULL) {
                        exit(1);
                }
                unsigned int i;
                for (i = 0; i < nstages; i++) {
                        struct result *r = &results[i];
                        r->solve_flags =
                                load_and_evaluate_stage(i + 1, &r->ev);
                }
                for (i = 0; i < nstages; i++) {
                        struct result *r = &results[i];
                        printf("stage %u solve_flags %u score %u\n", i + 1,
                               r->solve_flags, r->ev.score);
                }
                free(results);
        }
}