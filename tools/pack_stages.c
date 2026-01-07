#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitbuf.h"
#include "bitin.h"
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

struct ctx {
        struct chuff ch;
        uint8_t chtable[CHUFF_TABLE_SIZE_MAX];
        uint8_t *hufftables[CHUFF_NTABLES];
        size_t hufftablesizes[CHUFF_NTABLES];
};

int
main(int argc, char **argv)
{
        size_t curoff = 0;
        size_t offset[nstages];
        size_t maxmlen = 0;

        struct ctx ctx;
        struct ctx mctx;
        chuff_init(&ctx.ch);
        chuff_init(&mctx.ch);
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                ctx.ch.context = 0;
                chuff_update(&ctx.ch, data, data_size);
                if (stage->message != NULL) {
                        size_t len = strlen(stage->message) + 1;
                        if (len > maxmlen) {
                                maxmlen = len;
                        }
                        mctx.ch.context = 0;
                        chuff_update(&mctx.ch, (const void *)stage->message,
                                     len);
                }
        }
        chuff_build(&ctx.ch);
        chuff_build(&mctx.ch);

        chuff_table(&ctx.ch, ctx.chtable, ctx.hufftables, ctx.hufftablesizes);
        chuff_table(&mctx.ch, mctx.chtable, mctx.hufftables,
                    mctx.hufftablesizes);

        printf("#include \"hstages.h\"\n");

        printf("const uint8_t stages_huff_data[] = {\n");
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];

                /*
                 * encode stage->data and then stage->message,
                 * without flushing the bitbuf.
                 */

                offset[i] = curoff;
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
                ctx.ch.context = 0;
                struct bitbuf os;
                bitbuf_init(&os);
                chuff_encode(&ctx.ch, data, data_size, &os);

                size_t msg_size = 0;
                if (stage->message != NULL) {
                        msg_size = strlen(stage->message) + 1;
                        assert(msg_size <= MSG_SIZE_MAX);

                        mctx.ch.context = 0;
                        chuff_encode(&mctx.ch, (const void *)stage->message,
                                     msg_size, &os);
                }
                bitbuf_flush(&os);
                const uint8_t *encoded = os.p;
                size_t encoded_len = os.datalen;

                printf("// stage %04u %zu+%zu -> %zu bytes (%.1f %%)\n", i + 1,
                       data_size, msg_size, encoded_len,
                       (float)encoded_len / (data_size + msg_size) * 100);
                curoff += encoded_len;
                unsigned int j;
                for (j = 0; j < encoded_len; j++) {
                        printf("%#02x,", (unsigned int)encoded[j]);
                }
                printf("\n");
#if 1 /* debug */
                struct bitin in;
                bitin_init(&in, encoded);
                uint8_t decoded[data_size];
                uint8_t chuff_ctx = 0;
                for (j = 0; j < data_size; j++) {
                        decoded[j] = chuff_ctx = huff_decode_sym(
                                &in, ctx.hufftables[chuff_ctx]);
                }
                assert(!memcmp(data, decoded, data_size));
#endif
                bitbuf_clear(&os);
        }
        printf("};\n");

        printf("const struct hstage packed_stages[] = {\n");
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];

                printf("\t[%u] = {\n", i);
                assert(offset[i] < 0x8000);
                if (stage->message != NULL) {
                        printf("\t\t.data_offset = %zu | "
                               "HSTAGE_HAS_MESSAGE,\n",
                               offset[i]);
                } else {
                        printf("\t\t.data_offset = %zu,\n", offset[i]);
                }
                printf("\t},\n");
        }
        printf("};\n");

        /*
         * note: for our stage data, 16-bit indexes are good enough.
         *
         * for arbitrary inputs, it might not fit into 16-bit.
         * however, such a large table does not fit into wasm4's
         * 64KB linear memory anyway.
         */

        {
                printf("const uint16_t stages_huff_table_idx[] = {\n");
                uint16_t idx = 0;
                for (i = 0; i < ctx.ch.ntables; i++) {
                        printf("%#04x,", idx);
                        size_t sz = ctx.hufftablesizes[i];
                        assert(sz <= UINT16_MAX - idx);
                        idx += sz;
                }
                printf("};\n");
                size_t htablesize = idx;
                printf("// table size %zu bytes\n", htablesize);
                printf("const uint8_t stages_huff_table[] = {\n");
                for (i = 0; i < htablesize; i++) {
                        printf("%#02x,", (unsigned int)ctx.chtable[i]);
                }
                printf("};\n");
        }

        {
                printf("const uint16_t stages_msg_huff_table_idx[] = {\n");
                uint16_t idx = 0;
                for (i = 0; i < mctx.ch.ntables; i++) {
                        printf("%#04x,", idx);
                        size_t sz = mctx.hufftablesizes[i];
                        assert(sz <= UINT16_MAX - idx);
                        idx += sz;
                }
                printf("};\n");
                size_t htablesize = idx;
                printf("// table size %zu bytes\n", htablesize);
                printf("const uint8_t stages_msg_huff_table[] = {\n");
                for (i = 0; i < htablesize; i++) {
                        printf("%#02x,", (unsigned int)mctx.chtable[i]);
                }
                printf("};\n");
        }

        printf("const unsigned int nstages = %u;\n", nstages);
        // printf("const unsigned int maxmlen = %zu;\n", maxmlen);

        exit(0);
}
