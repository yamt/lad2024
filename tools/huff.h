/*
 * a straightforward static huffman encoder.
 *
 * assumptions:
 *
 * - an application only needs to embed a decoder.
 *   it's important to have a small decoder.
 *
 * - just huffman encoding is good enough for the application.
 *   (eg. no extra compression algorithms like RLE or LZ-thingy)
 *
 * - encoding is done offline. thus its speed is not important.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hnode {
        size_t count;
        union {
                struct leaf {
                        uint16_t encoded_nbits;
                        uint8_t encoded_bits[256 / 8];
                } leaf;
                struct inner {
                        struct hnode *children[2];
                } inner;
        } u;
};

struct hufftree {
        /*
         * 256 leaf nodes
         * 255 inner nodes
         * the last inner node is the root node
         *
         * [0]..[255]           - leaf nodes
         * [256]..[256 * 2 - 3] - inner nodes
         * [256 * 2 - 2]        - root node
         */
        struct hnode nodes[256 * 2 - 1];
};

/*
 * build a tree.
 * 1. call huff_init
 * 2. call huff_update multiple times to cover the whole data
 * 3. call huff_build to finalize the tree
 */
void huff_init(struct hufftree *tree);
void huff_update(struct hufftree *tree, const uint8_t *p, size_t len);
void huff_build(struct hufftree *tree);

/*
 * encode data using the tree built with using the above functions.
 *
 * the data to be encoded is expected to be same as what has been fed into
 * the tree with huff_update.
 * (at least the set of possible byte values in the data should be the same.)
 */
void huff_encode(const struct hufftree *tree, const uint8_t *p, size_t len,
                 uint8_t *out, size_t *lenp);
const uint8_t *huff_encode_byte(const struct hufftree *tree, uint8_t c,
                                uint16_t *nbitsp);

/*
 * serialize the tree for the decoder. (huff_decode.c)
 */
#define HUFF_TABLE_SIZE_MAX (255 * 3)
void huff_table(const struct hufftree *tree, uint8_t *out, size_t *lenp);
