#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test_util.h"

#include "bitin.h"
#include "huff.h"
#include "huff_debug.h"
#include "huff_decode.h"

int
main(void)
{
        size_t inputsize;
        uint8_t *input = read_fd(STDIN_FILENO, &inputsize);
        printf("input size: %zu bytes\n", inputsize);

        struct hufftree t;
        huff_init(&t);
        huff_update(&t, input, inputsize);
        huff_build(&t);

#if 0
        {
                printf("encoded bits for each values:\n");
                unsigned int i;
                for (i = 0; i < HUFF_NSYMS; i++) {
                        const struct hnode *n = &t.nodes[i];
                        uint16_t nbits = n->u.leaf.encoded_nbits;
                        if (nbits == 0) {
                                continue;
                        }
                        size_t count = n->count;
                        printf("    %02x: %3u bits (%7.3f %%)\n", i, nbits,
                               (double)count / inputsize * 100);
                }
        }
#endif

        uint8_t *encbuf = malloc(inputsize);
        if (encbuf == NULL) {
                fprintf(stderr, "malloc failed\n");
                exit(1);
        }
        size_t encsize;
        huff_encode(&t, input, inputsize, encbuf, &encsize);
        printf("encoded size: %zu bytes\n", encsize);
        assert(encsize <= inputsize);

        uint8_t htable[HUFF_TABLE_SIZE_MAX];
        size_t htablesize;
        huff_table(&t, htable, &htablesize);
        printf("table size: %zu bytes\n", htablesize);
        assert(htablesize <= sizeof(htable));

#if 0
        printf("tree dump:\n");
        huff_dump_tree(&t);
        printf("table dump:\n");
        huff_dump_table(htable, htablesize);
#endif

        printf("total compression ratio: (%zu + %zu) / %zu = %.4f\n", encsize,
               htablesize, inputsize,
               (double)(encsize + htablesize) / inputsize);

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
}
