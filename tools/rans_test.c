#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rans_common.h"
#include "rans_decode.h"
#include "rans_encode.h"
#include "rans_probs.h"

#include "bitbuf.h"
#include "bitin.h"
#include "byteout.h"

#include "test_util.h"

/* --- test */

static void
test_encode(rans_I extra, const void *input, size_t inputsize,
            const struct rans_probs *ps, struct bitbuf *bo)
{
        printf("encoding...\n");
        struct rans_encode_state st0;
        struct rans_encode_state *st = &st0;

        rans_encode_init(st);
        rans_encode_set_extra(st, extra);
        size_t i = inputsize;
        while (1) {
                i--;
                uint8_t sym = ((const uint8_t *)input)[i];
                rans_prob_t b_s = rans_b(ps->ls, sym);
                rans_prob_t l_s = ps->ls[sym];
                rans_encode_sym(st, sym, b_s, l_s, bo);
                if (i == 0) {
                        break;
                }
        }
        rans_encode_flush(st, bo);
}

static rans_I
test_decode(const void *input, size_t inputsize, size_t origsize,
            const rans_prob_t *ps, const rans_sym_t *trans, struct byteout *bo)
{
        printf("decoding...\n");
        struct rans_decode_state st0;
        struct rans_decode_state *st = &st0;

#if defined(RANS_DECODE_BITS)
        struct bitin in;
        bitin_init(&in, input);
#else
        const uint8_t *cp = input;
#endif

        rans_decode_init(st);
        while (bo->actual < origsize) {
#if defined(RANS_DECODE_BITS)
                rans_sym_t sym = rans_decode_sym(st, ps, &in);
#else
                rans_sym_t sym = rans_decode_sym(st, ps, &cp);
#endif
                if (trans != NULL) {
                        sym = trans[sym];
                }
                byteout_write(bo, sym);
        }
#if defined(RANS_DECODE_BITS)
        return rans_decode_get_extra(st, &in);
#else
        return rans_decode_get_extra(st, &cp);
#endif
}

static void
test(void)
{
        size_t inputsize;
        const uint8_t *input = read_fd(STDIN_FILENO, &inputsize);

        if (inputsize == 0) {
                printf("zero byte input\n");
                exit(0);
        }

        size_t counts[RANS_NSYMS];
        memset(counts, 0, sizeof(counts));
        count_syms(counts, input, inputsize);
        double bits = calc_bits(counts, calc_sum(counts), counts);
        printf("input entropy %.6f bits\n", bits);

        struct rans_probs ps;
        rans_probs_init(&ps, counts);

        rans_I extra = 0;

        struct bitbuf bo;
        bitbuf_init(&bo);
        test_encode(extra, input, inputsize, &ps, &bo);
        bitbuf_rev_flush(&bo);

        rans_prob_t table[RANS_TABLE_MAX_NELEMS];
        size_t tablesize;
        rans_probs_table(&ps, table, &tablesize);

        size_t ctablesize;
        rans_prob_t ctable[RANS_TABLE_MAX_NELEMS];
        rans_sym_t ctrans[RANS_NSYMS];
        rans_probs_table_with_trans(&ps, ctable, ctrans, &ctablesize);

        rans_I dextra;

        struct byteout bo_dec;
        byteout_init(&bo_dec);
        dextra =
                test_decode(bo.p, bo.datalen, inputsize, table, NULL, &bo_dec);
        assert(extra == dextra);
        assert(bo_dec.actual == inputsize);
        assert(!memcmp(bo_dec.p, input, inputsize));
        byteout_clear(&bo_dec);

        byteout_init(&bo_dec);
        dextra = test_decode(bo.p, bo.datalen, inputsize, ctable, ctrans,
                             &bo_dec);
        assert(extra == dextra);
        assert(bo_dec.actual == inputsize);
        assert(!memcmp(bo_dec.p, input, inputsize));
        byteout_clear(&bo_dec);

        printf("decoded correctly\n");
        printf("compression %zu -> %zu (%zu bits, %+.3f) + %zu\n", inputsize,
               bo.datalen, bo.datalen_bits, bo.datalen_bits - bits,
               tablesize * sizeof(rans_prob_t));
        printf("compression %zu -> %zu (%zu bits, %+.3f) + %zu + %zu (w/ "
               "trans)\n",
               inputsize, bo.datalen, bo.datalen_bits, bo.datalen_bits - bits,
               ctablesize * sizeof(rans_prob_t),
               ctablesize * sizeof(rans_sym_t));

        bitbuf_clear(&bo);
}

int
main(void)
{
        test();
}
