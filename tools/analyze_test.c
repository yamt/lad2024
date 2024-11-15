#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
#include "loader.h"
#include "maputil.h"
#include "stages.h"

/* clang-format off */

const struct stage solvable_stages[] = {
    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, END,
            _, W, X, _, _, _, X, W, END,
            W, _, W, _, A, _, W, _, W, END,
            W, _, _, _, A, _, _, _, W, END,
            W, _, _, _, U, _, _, _, W, END,
            _, W, _, _, P, _, _, W, END,
            _, _, W, W, W, W, W, END,
            END,
        },
	},
};

const struct stage unsolvable_stages[] = {

    /* direction of lights */
    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, END,
            _, W, _, _, W, _, X, W, END,
            W, _, _, D, A, _, _, W, W, END,
            W, _, _, _, _, _, L, _, W, END,
            W, _, _, _, P, _, _, _, W, END,
            _, W, _, _, _, _, _, W, END,
            _, _, W, W, W, W, W, END,
            END,
        },
	},

    /* can not collect both of Xs */
    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, END,
            _, W, X, _, _, _, X, W, END,
            W, _, W, _, A, _, W, _, W, END,
            W, _, _, _, _, _, _, _, W, END,
            W, _, _, _, U, _, _, _, W, END,
            _, W, _, _, P, _, _, W, END,
            _, _, W, W, W, W, W, END,
            END,
        },
	},
};

/* clang-format on */

#define ARRAYCOUNT(a) (sizeof(a) / sizeof(a[0]))

int
main(int argc, char **argv)
{
        /* all stages should be solvable */
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                map_t map;
                struct map_info info;
                decode_stage(i, map, &info);
                if (tsumi(map)) {
                        printf("unexpected tsumi on stage %03u\n", i + 1);
                        dump_map(map);
                        exit(1);
                }
        }

        for (i = 0; i < ARRAYCOUNT(solvable_stages); i++) {
                map_t map;
                struct map_info info;
                decode_stage_from(&solvable_stages[i], map, &info);
                if (tsumi(map)) {
                        printf("unexpected true from tsumi()\n");
                        dump_map(map);
                        exit(1);
                }
        }

        for (i = 0; i < ARRAYCOUNT(unsolvable_stages); i++) {
                map_t map;
                struct map_info info;
                decode_stage_from(&unsolvable_stages[i], map, &info);
                if (!tsumi(map)) {
                        printf("unexpected false from tsumi()\n");
                        dump_map(map);
                        exit(1);
                }
        }

        exit(0);
}
