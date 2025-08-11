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
load_and_evaluate_stage(unsigned int stage_number, struct evaluation *ev,
                        unsigned int *nmoves, struct solver_stat *stat,
                        struct map_info *info)
{
        map_t map;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, map, info);
        if (info->w > map_width || info->h > map_height) {
                printf("info %u %u\n", info->w, info->h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        dump_map(map);

        struct solution solution;
        unsigned int result =
                solve("solving", map, &solver_default_param, true, &solution);
        if (result == SOLVE_SOLVED) {
                evaluate(map, &solution.moves, ev);
        }
        if (result == SOLVE_SOLVED || result == SOLVE_SOLVABLE) {
                *nmoves = solution.nmoves;
        }
        *stat = solution.stat;
        clear_solution(&solution);
        solve_cleanup();
        return result;
}

int
main(int argc, char **argv)
{
        if (argc != 2 && argc != 3) {
                exit(2);
        }
        int stage_number = atoi(argv[1]);
        int stage_number_max;
        if (stage_number < 0 || stage_number > nstages) {
                exit(2);
        }
        if (argc == 3) {
                stage_number_max = atoi(argv[2]);
                if (stage_number_max < 0) {
                        exit(2);
                }
                if (stage_number_max > nstages) {
                        stage_number_max = nstages;
                }
        } else {
                stage_number_max = stage_number;
        }
        if (stage_number == 0) {
                stage_number = 1;
        }
        if (stage_number_max == 0) {
                stage_number_max = nstages;
        }

        node_allocator_init();
        struct result {
                unsigned int solve_flags;
                unsigned int nmoves;
                struct solver_stat stat;
                struct evaluation ev;
                struct map_info info;
        } * results;
        results = calloc(sizeof(*results), nstages);
        if (results == NULL) {
                exit(1);
        }
        unsigned int i;
        for (i = stage_number - 1; i < stage_number_max; i++) {
                struct result *r = &results[i];
                r->solve_flags = load_and_evaluate_stage(
                        i + 1, &r->ev, &r->nmoves, &r->stat, &r->info);
                if (r->solve_flags == SOLVE_IMPOSSIBLE) {
                        printf("stage %u is impossible!\n", i + 1);
                        exit(1);
                }
                if (((i + 1) % 50) != 0 && (i + 1 != stage_number_max)) {
                        continue;
                }
                printf("=============\n");
                unsigned int j;
                for (j = stage_number - 1; j <= i; j++) {
                        struct result *r = &results[j];
                        float node_memory_mb = (float)sizeof(struct node) *
                                               r->stat.nodes / 1024 / 1024;
                        printf("stage %03u w %2u h %2u solve_flags %u score "
                               "%4u nmoves %3u nodes %u (%.1f MB) iterations "
                               "%u\n",
                               j + 1, r->info.h, r->info.w, r->solve_flags,
                               r->ev.score, r->nmoves, r->stat.nodes,
                               node_memory_mb, r->stat.iterations);
                }
                printf("=============\n");
        }
        free(results);
}
