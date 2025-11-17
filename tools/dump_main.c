#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "dump.h"
#include "loader.h"

void
load_and_dump_stage(unsigned int stage_number)
{
        map_t map;
        struct map_info info;
        printf("    /* stage %u */\n", stage_number);
        decode_stage(stage_number - 1, map, &info);
        if (info.w > map_width || info.h > map_height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        dump_map_c_to(map, stdout);
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

        unsigned int i;
        for (i = stage_number - 1; i < stage_number_max; i++) {
                load_and_dump_stage(i + 1);
        }
}
