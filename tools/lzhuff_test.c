#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bitbuf.h"
#include "lzhuff.h"

#include "bitin.h"
#include "lz_decode.h"
#include "lzhuff_decode.h"

int
main(void)
{
        uint8_t *input = NULL;
        size_t inputsize = 0;
        size_t bufsz = 16;
        while (1) {
                input = realloc(input, bufsz);
                if (input == NULL) {
                        fprintf(stderr, "realloc failed\n");
                        exit(1);
                }
                ssize_t nread = read(STDIN_FILENO, &input[inputsize],
                                     bufsz - inputsize);
                if (nread == 0) {
                        break;
                }
                if (nread == -1) {
                        fprintf(stderr, "read failed\n");
                        exit(1);
                }
                inputsize += nread;
                assert(inputsize <= bufsz);
                if (inputsize == bufsz) {
                        bufsz *= 2;
                }
        }

        printf("buffer size: %zu bytes\n", bufsz);
        printf("input size: %zu bytes\n", inputsize);

#if HUFF_NSYMS < 256 + 1 + (MATCH_LEN_MAX - MATCH_LEN_MIN)
#warning HUFF_NSYMS seems too small
        huff_sym_t match_base = 0xb0; /* XXX */
#else
        huff_sym_t match_base = 256;
#endif
        assert(match_base + 1 + MATCH_LEN_MAX - MATCH_LEN_MIN <= HUFF_NSYMS);
        assert(1 + MATCH_DISTANCE_MAX - MATCH_DISTANCE_MIN <= HUFF_NSYMS);
        struct lzhuff lzh;
        lzhuff_init(&lzh, match_base);
        lzhuff_update(&lzh, input, inputsize);
        lzhuff_build(&lzh);

        struct bitbuf os;
        bitbuf_init(&os);
        lzhuff_encode_init(&lzh, &os);
        lzhuff_encode(&lzh, input, inputsize);
        lzhuff_flush(&lzh);
        bitbuf_flush(&os);
        size_t encsize = os.datalen;
        printf("encoded size: %zu bytes\n", encsize);

        uint8_t lhtable[HUFF_TABLE_SIZE_MAX];
        size_t lhtablesize;
        uint8_t dhtable[HUFF_TABLE_SIZE_MAX];
        size_t dhtablesize;
        huff_table(&lzh.huff_lit, lhtable, &lhtablesize);
        huff_table(&lzh.huff_dist, dhtable, &dhtablesize);
        assert(lhtablesize <= sizeof(lhtable));
        assert(dhtablesize <= sizeof(dhtable));
        printf("table size: %zu + %zu bytes\n", lhtablesize, dhtablesize);
        size_t htablesize = lhtablesize + dhtablesize;

        printf("total compression ratio: (%zu + %zu) / %zu = %.4f\n", encsize,
               htablesize, inputsize,
               (double)(encsize + htablesize) / inputsize);

        printf("test decoding...\n");
        struct lz_decode_state dec;
        lz_decode_init(&dec);
        struct bitin in;
        bitin_init(&in, os.p);
        size_t i;
        for (i = 0; i < inputsize; i++) {
                uint8_t actual = lzhuff_decode_sym(&dec, &in, lhtable, dhtable,
                                                   match_base);
                assert(in.p >= os.p);
                assert(in.p < os.p + encsize ||
                       (in.p == os.p + encsize && in.bitoff == 0));
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
