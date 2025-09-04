#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "dump.h"
#include "list.h"
#include "loader.h"
#include "maputil.h"
#include "node.h"
#include "refine.h"
#include "solver.h"

unsigned int
load_and_refine_stage(unsigned int stage_number)
{
        map_t map;
        struct map_info info;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, map, &info);
        if (info.w > map_width || info.h > map_height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        struct solution solution;
        unsigned int result =
                solve("solving", map, &solver_default_param, true, &solution);
        map_t orig;
        map_copy(orig, map);
        if (result == SOLVE_SOLVED &&
            try_refine(map, &solution, &solver_default_param)) {
                printf("refined!\n");
                dump_map(orig);
                dump_map(map);
                dump_map_c_fmt(map, "refined-%03u.c", stage_number);
        } else {
                printf("nothing to refine\n");
        }
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
                if (stage_number_max < 0 || stage_number > nstages) {
                        exit(2);
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
        unsigned int i;
        for (i = stage_number - 1; i < stage_number_max; i++) {
                unsigned int result = load_and_refine_stage(i + 1);
                if (result == SOLVE_IMPOSSIBLE) {
                        printf("stage %u is impossible!\n", i + 1);
                        exit(1);
                }
        }
}
