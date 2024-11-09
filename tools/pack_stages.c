#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "huff.h"
#include "huff_decode.h"
#include "loader.h"
#include "maputil.h"

int
main(int argc, char **argv)
{
        struct hufftree huff;
        huff_init(&huff);
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                map_t map;
                struct map_info info;
                decode_stage(i, map, &info);
                huff_update(&huff, map, map_size);
        }
        huff_build(&huff);

        uint8_t htable[HUFF_TABLE_SIZE_MAX];
        size_t htablesize;
        huff_table(&huff, htable, &htablesize);

        for (i = 0; i < nstages; i++) {
                map_t map;
                struct map_info info;
                decode_stage(i, map, &info);
                uint8_t encoded[map_size];
                size_t encoded_len;
                huff_encode(&huff, map, map_size, encoded, &encoded_len);
                printf("stage %03u %zu bytes (%.1f %%)\n", i + 1, encoded_len,
                       (float)encoded_len / map_size * 100);

                struct huff_decode_context ctx;
                huff_decode_init(&ctx, encoded);
                map_t map2;
                loc_t loc;
                for (loc = 0; loc < map_size; loc++) {
                        map2[loc] = huff_decode_byte(&ctx, htable);
                }
                assert(!memcmp(map, map2, map_size));
        }
        exit(0);
}
