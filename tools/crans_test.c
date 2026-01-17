#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test_util.h"

#include "bitbuf.h"
#include "crans.h"

#include "crans_decode.h"

int
main(void)
{
        size_t inputsize;
        uint8_t *input = read_fd(STDIN_FILENO, &inputsize);
        printf("input size: %zu bytes\n", inputsize);

        struct crans t;
        crans_init(&t);
        t.context = 0;
        crans_update(&t, input, inputsize);
        crans_build(&t);

		struct rans_encode_state enc;
        rans_encode_init(&enc);
        struct bitbuf os;
        bitbuf_init(&os);
        t.context = 0;
        crans_encode(&t, input, inputsize, &enc, &os);
        rans_encode_flush(&enc, &os);
        bitbuf_rev_flush(&os);
        size_t encsize = os.datalen;
        printf("encoded size: %zu bytes\n", encsize);

        rans_prob_t htable[CRANS_TABLE_MAX_NELEMS];
        rans_prob_t *outs[CRANS_NTABLES];
        size_t lens[CRANS_NTABLES];
        uint16_t indexes[CRANS_NTABLES];
        crans_table(&t, htable, outs, lens);
        size_t htablesize = 0;
        size_t i;
        for (i = 0; i < t.ntables; i++) {
                indexes[i] = htablesize;
                htablesize += lens[i];
        }
        printf("table size: %zu bytes\n", htablesize * sizeof(rans_prob_t));
        assert(htablesize <= sizeof(htable));

        printf("total compression ratio: (%zu + %zu) / %zu = %.4f\n", encsize,
               htablesize * sizeof(rans_prob_t) , inputsize,
               (double)(encsize + htablesize * sizeof(rans_prob_t)) / inputsize);

        printf("test decoding...\n");
        struct crans_decode_context dec;
        dec.ctx = 0;
        rans_decode_init(&dec.dec);
#if defined(RANS_DECODE_BITS)
        bitin_init(&dec.inp, os.p);
#else
		dec.inp = os.p;
#endif
        dec.ctx = 0;
        for (i = 0; i < inputsize; i++) {
                uint8_t actual = crans_decode_byte(&dec, htable, NULL, indexes);
                uint8_t expected = input[i];
                if (actual != expected) {
                        printf("unexpected data at offset %zu, expected %02x, "
                               "actual %02x\n",
                               i, expected, actual);
                        abort();
                }
        }
        printf("successfully decoded\n");
        bitbuf_clear(&os);
}
