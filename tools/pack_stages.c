#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chuff.h"
#include "defs.h"
#include "dump.h"
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
        size_t curoff = 0;
        size_t offset[nstages];

        struct chuff ch;
        chuff_init(&ch);
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                ch.context = 0;
                chuff_update(&ch, data, data_size);
        }
        chuff_build(&ch);

        uint8_t chtable[CHUFF_TABLE_SIZE_MAX];
        uint8_t *hufftables[CHUFF_NTABLES];
        size_t hufftablesizes[CHUFF_NTABLES];
        chuff_table(&ch, chtable, hufftables, hufftablesizes);
        assert(chtable == hufftables[0]);
        assert(hufftables[1] == hufftables[0] + hufftablesizes[0]);
        assert(hufftables[2] == hufftables[1] + hufftablesizes[1]);

        printf("#include \"hstages.h\"\n");

        printf("const uint8_t stages_huff_data[] = {\n");
        for (i = 0; i < nstages; i++) {
                offset[i] = curoff;
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                uint8_t encoded[data_size];
                size_t encoded_len;
                ch.context = 0;
                chuff_encode(&ch, data, data_size, encoded, &encoded_len);
                printf("// stage %03u %zu bytes (%.1f %%)\n", i + 1,
                       encoded_len, (float)encoded_len / data_size * 100);
                curoff += encoded_len;

                unsigned int j;
#if 1 /* debug */
                struct huff_decode_context ctx;
                huff_decode_init(&ctx, encoded);
                uint8_t decoded[data_size];
                uint8_t chuff_ctx = 0;
                for (j = 0; j < data_size; j++) {
                        decoded[j] = chuff_ctx =
                                huff_decode_byte(&ctx, hufftables[chuff_ctx]);
                }
                assert(!memcmp(data, decoded, data_size));
#endif
                for (j = 0; j < encoded_len; j++) {
                        printf("%#02x,", (unsigned int)encoded[j]);
                }
                printf("\n");
        }
        printf("};\n");

        printf("const struct hstage packed_stages[] = {\n");
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];

                printf("\t[%u] = {\n", i);
                printf("\t\t.data_offset = %zu,\n", offset[i]);
                if (stage->message != NULL) {
                        printf("\t\t.message = (const uint8_t[]){\n");

                        const char *cp = stage->message;
                        char ch;
                        do {
                                ch = *cp++;
                                printf("%#02x,", (unsigned int)(uint8_t)ch);
                        } while (ch != 0);
                        printf("\n");

                        printf("\t\t},\n");
                }
                printf("\t},\n");
        }
        printf("};\n");

        printf("const uint16_t stages_huff_table_idx[] = {\n");
        uint16_t idx = 0;
        for (i = 0; i < ch.ntables; i++) {
                printf("%#04x,", idx);
                size_t sz = hufftablesizes[i];
                assert(sz <= UINT16_MAX - idx);
                idx += sz;
        }
        printf("};\n");
        size_t htablesize = idx;
        printf("// table size %zu bytes\n", htablesize);
        printf("const uint8_t stages_huff_table[] = {\n");
        for (i = 0; i < htablesize; i++) {
                printf("%#02x,", (unsigned int)chtable[i]);
        }
        printf("};\n");

        printf("const unsigned int nstages = %u;\n", nstages);

        exit(0);
}
