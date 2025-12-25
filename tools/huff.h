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
                        uint8_t encoded_nbits;
                        uint16_t encoded_bits;
                } leaf;
                struct inner {
                        struct hnode *children[2];
                } inner;
        } u;
};

struct hufftree {
        /*
         * [0]..[255]           - leaf nodes
         * [256]..[256 * 2 - 3] - inner nodes
         * [256 * 2 - 2]        - root node
         */
        struct hnode nodes[256 * 2 - 1];
};

void huff_init(struct hufftree *tree);
void huff_update(struct hufftree *tree, const uint8_t *p, size_t len);
void huff_build(struct hufftree *tree);

void huff_encode(const struct hufftree *tree, const uint8_t *p, size_t len,
                 uint8_t *out, size_t *lenp);
uint16_t huff_encode_byte(const struct hufftree *tree, uint8_t c,
                          uint8_t *nbitsp);
/* a tree with no huff_update() called is "empty" */
bool huff_is_empty(const struct hufftree *tree);

#define HUFF_TABLE_SIZE_MAX (255 * 3)
void huff_table(const struct hufftree *tree, uint8_t *out, size_t *lenp);
