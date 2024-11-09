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
#include "stages.h"

size_t
stage_data_size(const uint8_t *p)
{
        const uint8_t *sp = p;
        uint8_t ch;
        while (true) {
                ch = *p++;
                if (ch == END) {
                        ch = *p++;
                        if (ch == END) {
                                return p - sp;
                        }
                }
        }
}

int
main(int argc, char **argv)
{
        struct hufftree huff;
        huff_init(&huff);
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                huff_update(&huff, data, data_size);
        }
        huff_build(&huff);

        uint8_t htable[HUFF_TABLE_SIZE_MAX];
        size_t htablesize;
        huff_table(&huff, htable, &htablesize);

        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                uint8_t encoded[data_size];
                size_t encoded_len;
                huff_encode(&huff, data, data_size, encoded, &encoded_len);
                printf("stage %03u %zu bytes (%.1f %%)\n", i + 1, encoded_len,
                       (float)encoded_len / data_size * 100);

                struct huff_decode_context ctx;
                huff_decode_init(&ctx, encoded);
                uint8_t decoded[data_size];
                unsigned int j;
                for (j = 0; j < data_size; j++) {
                        decoded[j] = huff_decode_byte(&ctx, htable);
                }
                assert(!memcmp(data, decoded, data_size));
        }
        exit(0);
}
