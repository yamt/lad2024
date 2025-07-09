#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "loader.h"
#include "maputil.h"

int
main(int argc, char **argv)
{
        struct stage {
                map_t map;
        } * stages;
        stages = calloc(nstages, sizeof(*stages));
        if (stages == NULL) {
                exit(1);
        }
        bool dup_found = false;
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                struct stage *stage = &stages[i];
                struct map_info info;
                decode_stage(i, stage->map, &info);
                unsigned int j;
                for (j = 0; j < i; j++) {
                        if (!memcmp(stage->map, stages[j].map, map_size)) {
                                printf("stage %u and %u are same\n", j + 1,
                                       i + 1);
                                dump_map(stage->map);
                                dup_found = true;
                        }
                }
        }
        if (!dup_found) {
                printf("all stages are uniq\n");
                exit(0);
        }
        exit(1);
}
