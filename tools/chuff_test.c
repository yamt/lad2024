#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bitbuf.h"
#include "chuff.h"

#include "chuff_decode.h"

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

        struct chuff t;
        chuff_init(&t);
        t.context = 0;
        chuff_update(&t, input, inputsize);
        chuff_build(&t);

        struct bitbuf os;
        bitbuf_init(&os);
        t.context = 0;
        chuff_encode(&t, input, inputsize, &os);
        bitbuf_flush(&os);
        size_t encsize = os.datalen;
        printf("encoded size: %zu bytes\n", encsize);
        assert(encsize <= inputsize);

        uint8_t htable[CHUFF_TABLE_SIZE_MAX];
        uint8_t *outs[CHUFF_NTABLES];
        size_t lens[CHUFF_NTABLES];
        uint16_t indexes[CHUFF_NTABLES];
        chuff_table(&t, htable, outs, lens);
        size_t htablesize = 0;
        size_t i;
        for (i = 0; i < t.ntables; i++) {
                indexes[i] = htablesize;
                htablesize += lens[i];
        }
        printf("table size: %zu bytes\n", htablesize);
        assert(htablesize <= sizeof(htable));

        printf("total compression ratio: (%zu + %zu) / %zu = %.4f\n", encsize,
               htablesize, inputsize,
               (double)(encsize + htablesize) / inputsize);

        printf("test decoding...\n");
        struct chuff_decode_context dec;
        bitin_init(&dec.in, os.p);
        dec.chuff_ctx = 0;
        for (i = 0; i < inputsize; i++) {
                uint8_t actual = chuff_decode_byte(&dec, htable, indexes);
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
