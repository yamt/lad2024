#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "loader.h"
#include "maputil.h"
#include "simplify.h"
#include "stages.h"

/* clang-format off */

const struct stage test_stages[] = {
    {
        .data = (const uint8_t[]){
            _,W,_,W,W,W, END,
            W,W,W,_,_,X,W, END,
            W,D,W,_,_,W,W,W, END,
            W,_,W,_,D,P,W,W, END,
            W,A,W,_,R,A,W,W, END,
            W,X,W,A,_,W,W, END,
            W,W,W,_,W, END,
            _,W,_,W, END,
            END, END,
        },
    },
};

const struct stage expected_stages[] = {
    {
        .data = (const uint8_t[]){
            _,W, END,
            W,D,W, END,
            W,_,W, END,
            W,A,W, END,
            W,X,W, END,
            _,W, END,
            END, END,
        },
    },
};

/* clang-format on */

#define ARRAYCOUNT(a) (sizeof(a) / sizeof(a[0]))

int
main(int argc, char **argv)
{
        unsigned int i;
        for (i = 0; i < ARRAYCOUNT(test_stages); i++) {
                map_t orig;
                map_t map;
                struct map_info info;
                decode_stage_from(&test_stages[i], map, &info);
                map_copy(orig, map);
                simplify(map);
                align_to_top_left(map);
                map_t emap;
                decode_stage_from(&expected_stages[i], emap, &info);
                if (memcmp(map, emap, map_size)) {
                        printf("mismatch\n");
                        printf("original:\n");
                        dump_map(orig);
                        printf("actual:\n");
                        dump_map(map);
                        printf("expeced:\n");
                        dump_map(emap);
                        exit(1);
                }
        }
        printf("success\n");
        exit(0);
}
