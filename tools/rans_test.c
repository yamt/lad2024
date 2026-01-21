/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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

enum mode {
        test_mode_normal,
        test_mode_prob,
        test_mode_zero,
};

static void
test_encode(const void *input, size_t inputsize, const struct rans_probs *ps,
            struct bitbuf *bo, enum mode mode)
{
        printf("encoding...(mode=%u)\n", mode);
        struct rans_encode_state st0;
        struct rans_encode_state *st = &st0;
        bool need_init = false;

        switch (mode) {
        case test_mode_normal:
                rans_encode_init(st);
                break;
        case test_mode_prob:
                need_init = true;
                break;
        case test_mode_zero:
                rans_encode_init_zero(st);
                break;
        }

        size_t i = inputsize;
        while (1) {
                i--;
                uint8_t sym = ((const uint8_t *)input)[i];
                rans_prob_t b_s = rans_b(ps->ls, sym);
                rans_prob_t l_s = ps->ls[sym];
                if (need_init) {
                        rans_encode_init_with_prob(st, l_s);
                        need_init = false;
                }
                rans_encode_sym(st, sym, b_s, l_s, bo);
                if (i == 0) {
                        break;
                }
        }
        rans_encode_flush(st, bo);
}

static rans_I
test_decode(const void *input, size_t inputsize_bits, size_t origsize,
            const rans_prob_t *ps, const rans_sym_t *trans, struct byteout *bo,
            enum mode mode)
{
        if (trans == NULL) {
                printf("decoding w/o trans...\n");
        } else {
                printf("decoding w/ trans...\n");
        }
        struct rans_decode_state st0;
        struct rans_decode_state *st = &st0;

        if (mode != test_mode_zero) {
                inputsize_bits = SIZE_MAX;
        }

#if defined(RANS_DECODE_BITS)
        struct bitin in;
        bitin_init(&in, input);
#else
        const uint8_t *in = input;
#endif

        rans_decode_init(st);
        while (bo->actual < origsize) {
                while (rans_decode_need_more(st) &&
                       inputsize_bits >= RANS_B_BITS) {
#if defined(RANS_DECODE_BITS)
                        uint16_t bits = bitin_get_bits(&in, RANS_B_BITS);
#else
                        uint8_t bits = *in++;
#endif
                        rans_decode_feed(st, bits);
                        inputsize_bits -= RANS_B_BITS;
                }
                rans_sym_t sym = rans_decode_sym(st, ps);
                if (trans != NULL) {
                        sym = trans[sym];
                }
                byteout_write(bo, sym);
        }
        while (rans_decode_need_more(st) && inputsize_bits >= RANS_B_BITS) {
#if defined(RANS_DECODE_BITS)
                uint16_t d = bitin_get_bits(&in, RANS_B_BITS);
#else
                uint8_t d = *in++;
#endif
                rans_decode_feed(st, d);
                inputsize_bits -= RANS_B_BITS;
        }
        return rans_decode_get_extra(st);
}

static void
test(const void *input, size_t inputsize, enum mode mode)
{
        size_t counts[RANS_NSYMS];
        memset(counts, 0, sizeof(counts));
        count_syms(counts, input, inputsize);
        double bits = calc_bits(counts, calc_sum(counts), counts);
        printf("input entropy %.3f bits\n", bits);

        struct rans_probs ps;
        rans_probs_init(&ps, counts);

        struct bitbuf bo;
        bitbuf_init(&bo);
        test_encode(input, inputsize, &ps, &bo, mode);
        bitbuf_rev_flush(&bo);

        rans_prob_t table[RANS_TABLE_MAX_NELEMS];
        size_t tablesize;
        rans_probs_table(&ps, table, &tablesize);

        size_t ctablesize;
        rans_prob_t ctable[RANS_TABLE_MAX_NELEMS];
        rans_sym_t ctrans[RANS_NSYMS];
        rans_probs_table_with_trans(&ps, ctable, ctrans, &ctablesize);

        struct byteout bo_dec;
        byteout_init(&bo_dec);
        test_decode(bo.p, bo.datalen_bits, inputsize, table, NULL, &bo_dec,
                    mode);
        assert(bo_dec.actual == inputsize);
        assert(!memcmp(bo_dec.p, input, inputsize));
        byteout_clear(&bo_dec);

        byteout_init(&bo_dec);
        test_decode(bo.p, bo.datalen_bits, inputsize, ctable, ctrans, &bo_dec,
                    mode);
        assert(bo_dec.actual == inputsize);
        assert(!memcmp(bo_dec.p, input, inputsize));
        byteout_clear(&bo_dec);

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
        printf("B=%u L=%u M=%u\n", RANS_B, RANS_L, RANS_M);
        size_t inputsize;
        const uint8_t *input = read_fd(STDIN_FILENO, &inputsize);
        if (inputsize == 0) {
                printf("zero byte input\n");
                exit(0);
        }

        test(input, inputsize, test_mode_normal);
        test(input, inputsize, test_mode_prob);
        test(input, inputsize, test_mode_zero);

        free((void *)input);
}
