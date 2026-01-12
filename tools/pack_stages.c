#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USE_CRANS

#if defined(USE_CRANS)
#include "byteout.h"
#include "crans.h"
#include "rans_decode.h"
#else
#include "bitbuf.h"
#include "bitin.h"
#include "chuff.h"
#include "huff_decode.h"
#endif

#include "defs.h"
#include "dump.h"
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
#if defined(USE_CRANS)
        struct crans ch;
        rans_prob_t table[CRANS_TABLE_MAX_NELEMS];
        rans_prob_t *tables[CRANS_NTABLES];
        size_t tablesizes[CRANS_NTABLES];
#else
        struct chuff ch;
        uint8_t table[CHUFF_TABLE_SIZE_MAX];
        uint8_t *tables[CHUFF_NTABLES];
        size_t tablesizes[CHUFF_NTABLES];
#endif
};

int
main(int argc, char **argv)
{
        size_t curoff = 0;
        size_t offset[nstages];
        size_t maxmlen = 0;

        struct ctx ctx;
        struct ctx mctx;
#if defined(USE_CRANS)
        crans_init(&ctx.ch);
        crans_init(&mctx.ch);
#else
        chuff_init(&ctx.ch);
        chuff_init(&mctx.ch);
#endif
        unsigned int i;
        for (i = 0; i < nstages; i++) {
                const struct stage *stage = &stages[i];
                const uint8_t *data = stage->data;
                size_t data_size = stage_data_size(data);
#if defined(USE_CRANS)
                crans_update(&ctx.ch, data, data_size);
#else
                ctx.ch.context = 0;
                chuff_update(&ctx.ch, data, data_size);
#endif
                if (stage->message != NULL) {
                        size_t len = strlen(stage->message) + 1;
                        if (len > maxmlen) {
                                maxmlen = len;
                        }
#if defined(USE_CRANS)
                        crans_update(&mctx.ch, (const void *)stage->message,
                                     len);
#else
                        mctx.ch.context = 0;
                        chuff_update(&mctx.ch, (const void *)stage->message,
                                     len);
#endif
                }
        }

        size_t msg_nsyms;
#if defined(USE_CRANS)
        crans_build(&ctx.ch);
        crans_build(&mctx.ch);

        crans_table(&ctx.ch, ctx.table, ctx.tables, ctx.tablesizes);
        static rans_sym_t msg_trans[CRANS_TABLE_MAX_NELEMS];
        crans_table_with_trans(&mctx.ch, mctx.table, msg_trans, mctx.tables,
                                      mctx.tablesizes);
        msg_nsyms = mctx.ch.ntables;
#else
        chuff_build(&ctx.ch);
        chuff_build(&mctx.ch);

        chuff_table(&ctx.ch, ctx.table, ctx.tables, ctx.tablesizes);
        chuff_table(&mctx.ch, mctx.table, mctx.tables, mctx.tablesizes);
        msg_nsyms = mctx.ch.ntables;
#endif

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
#if defined(USE_CRANS)
                struct rans_encode_state enc;
                rans_encode_init(&enc);
                struct byteout bo;
                byteout_init(&bo);
#else
                ctx.ch.context = 0;
                struct bitbuf os;
                bitbuf_init(&os);
                chuff_encode(&ctx.ch, data, data_size, &os);
#endif

                size_t msg_size = 0;
                if (stage->message != NULL) {
                        msg_size = strlen(stage->message) + 1;
                        assert(msg_size <= MSG_SIZE_MAX);

#if defined(USE_CRANS)
                        crans_encode(&mctx.ch, (const void *)stage->message,
                                     msg_size, &enc, &bo);
#else
                        mctx.ch.context = 0;
                        chuff_encode(&mctx.ch, (const void *)stage->message,
                                     msg_size, &os);
#endif
                }
#if defined(USE_CRANS)
                /*
                 * note: encode in the reversed order for rANS
                 * (message and then data)
                 */
                crans_encode(&ctx.ch, data, data_size, &enc, &bo);
                rans_encode_flush(&enc, &bo);
                const uint8_t *encoded = rev_byteout_ptr(&bo);
                size_t encoded_len = bo.actual;
#else
                bitbuf_flush(&os);
                const uint8_t *encoded = os.p;
                size_t encoded_len = os.datalen;
#endif

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
#if defined(USE_CRANS)
                uint8_t decoded[data_size];
                struct rans_decode_state dec;
                rans_decode_init(&dec);
                const uint8_t *inp = encoded;
                uint8_t ch = 0;
                for (j = 0; j < data_size; j++) {
                        ch = rans_decode_sym(&dec, ctx.tables[ch], &inp);
                        decoded[j] = ch;
                }
                assert(!memcmp(data, decoded, data_size));

                if (stage->message != NULL) {
                        uint8_t mdecoded[msg_size];
                        uint8_t ch = 0;
                        for (j = 0; j < msg_size; j++) {
                                unsigned int off = mctx.tables[ch] - mctx.table;
                                size_t sz = mctx.tablesizes[ch];
                                assert(sz > 0);
                                ch = rans_decode_sym(&dec, mctx.tables[ch],
                                                     &inp);
                                assert(ch < sz);
                                mdecoded[j] = ch = msg_trans[off + ch];
                        }
                        assert(!memcmp(stage->message, mdecoded, msg_size));
                }
#else
                struct bitin in;
                bitin_init(&in, encoded);
                uint8_t decoded[data_size];
                uint8_t chuff_ctx = 0;
                for (j = 0; j < data_size; j++) {
                        decoded[j] = chuff_ctx =
                                huff_decode_sym(&in, ctx.tables[chuff_ctx]);
                }
                assert(!memcmp(data, decoded, data_size));
#endif
#endif
#if defined(USE_CRANS)
                byteout_clear(&bo);
#else
                bitbuf_clear(&os);
#endif
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
                        printf("%u,", idx);
                        size_t sz = ctx.tablesizes[i];
                        assert(sz <= UINT16_MAX - idx);
                        idx += sz;
                }
                printf("};\n");
                size_t htablesize = idx;
#if defined(USE_CRANS)
                printf("// table size %zu bytes\n",
                       htablesize * sizeof(rans_prob_t));
                printf("const rans_prob_t stages_huff_table[] = {\n");
#else
                printf("// table size %zu bytes\n", htablesize);
                printf("const uint8_t stages_huff_table[] = {\n");
#endif
                for (i = 0; i < htablesize; i++) {
#if defined(USE_CRANS)
                        printf("%#04x,", (unsigned int)ctx.table[i]);
#else
                        printf("%#02x,", (unsigned int)ctx.table[i]);
#endif
                }
                printf("};\n");
        }

        {
                printf("const uint16_t stages_msg_huff_table_idx[] = {\n");
                uint16_t idx = 0;
                for (i = 0; i < msg_nsyms; i++) {
                        printf("%u,", idx);
                        size_t sz = mctx.tablesizes[i];
                        assert(sz <= UINT16_MAX - idx);
                        idx += sz;
                }
                printf("};\n");
                size_t htablesize = idx;
#if defined(USE_CRANS)
                printf("// table size %zu bytes\n",
                       htablesize * sizeof(rans_prob_t));
                printf("const rans_prob_t stages_msg_huff_table[] = {\n");
#else
                printf("// table size %zu bytes\n", htablesize);
                printf("const uint8_t stages_msg_huff_table[] = {\n");
#endif
                for (i = 0; i < htablesize; i++) {
#if defined(USE_CRANS)
                        printf("%#04x,", (unsigned int)mctx.table[i]);
#else
                        printf("%#02x,", (unsigned int)mctx.table[i]);
#endif
                }
                printf("};\n");

#if defined(USE_CRANS)
                printf("// msg trans table size %zu bytes\n", htablesize);
                printf("const rans_sym_t stages_msg_trans[] = {\n");
                for (i = 0; i < htablesize; i++) {
                        printf("%#02x,", (unsigned int)msg_trans[i]);
                }
                printf("};\n");
#endif
        }

        printf("const unsigned int nstages = %u;\n", nstages);
        // printf("const unsigned int maxmlen = %zu;\n", maxmlen);

        exit(0);
}
