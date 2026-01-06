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

        struct lzhuff lzh;
        lzhuff_init(&lzh, 128); /* XXX */
        lzhuff_update(&lzh, input, inputsize);
        lzhuff_build(&lzh);

		struct bitbuf os;
        bitbuf_init(&os);
		lzhuff_encode_init(&lzh, &os);
        lzhuff_encode(&lzh, input, inputsize);
        lzhuff_encode_flush(&lzh);
        bitbuf_flush(&os);
        size_t encsize = os.datalen;
        printf("encoded size: %zu bytes\n", encsize);

#if 0
        uint8_t htable[HUFF_TABLE_SIZE_MAX];
        size_t htablesize;
        huff_table(&t, htable, &htablesize);
        printf("table size: %zu bytes\n", htablesize);
        assert(htablesize <= sizeof(htable));
#else
        size_t htablesize = 0; /* XXX */
#endif

        printf("total compression ratio: (%zu + %zu) / %zu = %.4f\n", encsize,
               htablesize, inputsize,
               (double)(encsize + htablesize) / inputsize);

#if 0
        printf("test decoding...\n");
        struct bitin dec;
        bitin_init(&dec, encbuf);
        size_t i;
        for (i = 0; i < inputsize; i++) {
                uint8_t actual = huff_decode_sym(&dec, htable);
                uint8_t expected = input[i];
                if (actual != expected) {
                        printf("unexpected data at offset %zu, expected %02x, "
                               "actual %02x\n",
                               i, expected, actual);
                        abort();
                }
        }
        printf("successfully decoded\n");
#endif
}
