#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bitbuf.h"
#include "huff.h"

static bool
is_leaf(const struct hufftree *tree, const struct hnode *n)
{
        return n - tree->nodes < 256;
}

static uint8_t
leaf_value(const struct hufftree *tree, const struct hnode *n)
{
        assert(is_leaf(tree, n));
        return n - tree->nodes;
}

static void
init_leaf_nodes(struct hufftree *tree)
{
}

void
huff_update(struct hufftree *tree, const uint8_t *p, size_t len)
{
        const uint8_t *cp = p;
        const uint8_t *ep = cp + len;
        while (cp < ep) {
                uint8_t u = *cp++;
                struct hnode *n = &tree->nodes[u];
                n->count++;
        }
}

static int
cmp_node(const void *a, const void *b)
{
        struct hnode *na = *(struct hnode **)a;
        struct hnode *nb = *(struct hnode **)b;
        if (na->count < nb->count) {
                return 1;
        }
        if (na->count > nb->count) {
                return -1;
        }
        return 0;
}

static void
build_tree(struct hufftree *tree)
{
        struct hnode *nodes[256];
        unsigned int i;
        for (i = 0; i < 256; i++) {
                nodes[i] = &tree->nodes[i];
        }
        for (i = 0; i < 256 - 1; i++) {
                /*
                 * REVISIT: it would be more efficient to use a priority queue.
                 */

                /*
                 * the number of valid elements in 'nodes'.
                 */
                const unsigned int n = 256 - i;
                /*
                 * pick the two nodes with the smallest count.
                 */
                qsort(nodes, n, sizeof(*nodes), cmp_node);
                struct hnode *inner = &tree->nodes[256 + i];
                struct hnode *na = nodes[n - 2];
                struct hnode *nb = nodes[n - 1];
                /*
                 * build a new inner node to represent the picked two nodes.
                 */
                inner->count = na->count + nb->count;
                inner->u.inner.children[0] = na;
                inner->u.inner.children[1] = nb;
                /*
                 * replace the two nodes with the inner node.
                 */
                nodes[n - 2] = inner;
                nodes[n - 1] = NULL; /* just in case */
        }
}

static void
finish_node(const struct hufftree *tree, struct hnode *n, unsigned int nbits,
            uint16_t bits)
{
        if (n->count == 0) {
                return;
        }
        if (is_leaf(tree, n)) {
#if 0
                printf("nbits %u value %02x\n", nbits,
                       (unsigned int)leaf_value(tree, n));
#endif
                n->u.leaf.encoded_nbits = nbits;
                n->u.leaf.encoded_bits = bits;
                return;
        }
        finish_node(tree, n->u.inner.children[0], nbits + 1, (bits << 1));
        finish_node(tree, n->u.inner.children[1], nbits + 1, (bits << 1) | 1);
}

/* this is a macro because it's used for both of const/non-const trees */
#define root_node(tree) (&(tree)->nodes[256 * 2 - 2])

static void
finish_tree(struct hufftree *tree)
{
        /*
         * assign encoded bits to leaf nodes.
         *
         * note: leaf nodes with count==0 are left uninitialized.
         * (thus u.leaf.encoded_bits==0)
         */
        finish_node(tree, root_node(tree), 0, 0);
}

void
huff_init(struct hufftree *tree)
{
        memset(tree, 0, sizeof(*tree));
        init_leaf_nodes(tree);
}

void
huff_build(struct hufftree *tree)
{
        build_tree(tree);
        finish_tree(tree);
}

uint16_t
huff_encode_byte(const struct hufftree *tree, uint8_t c, uint8_t *nbitsp)
{
        const struct hnode *n = &tree->nodes[c];
        assert(n->count > 0); /* "c" hasn't fed into huff_update? */
        assert(is_leaf(tree, n));
        uint16_t bits = n->u.leaf.encoded_bits;
        uint8_t nbits = n->u.leaf.encoded_nbits;
        assert(nbits > 0);
        *nbitsp = nbits;
        return bits;
}

/*
 * huffman-encoded data encoding:
 *
 * each bits in the encoded stream is an index into the table.
 * the last byte is padded with zero up to the next byte boundary.
 */

void
huff_encode(const struct hufftree *tree, const uint8_t *p, size_t len,
            uint8_t *out, size_t *lenp)
{
        const uint8_t *cp = p;
        const uint8_t *ep = cp + len;
        uint8_t *outp = out;
        struct bitbuf os;
        bitbuf_init(&os);
        while (cp < ep) {
                uint8_t nbits;
                uint16_t bits = huff_encode_byte(tree, *cp++, &nbits);
                bitbuf_write(&os, &outp, bits, nbits);
        }
        bitbuf_flush(&os, &outp);
        assert(outp - out <= len);
        *lenp = outp - out;
}

/*
 * table encoding:
 *
 * a table is an array of 3-byte entries.
 * an entry describes an inner or root node.
 * (thus a table has up to 255 nodes)
 * the entry for the root node is of the index 0. (the first one)
 *
 * an entry is:
 *   byte 0:
 *     bit 0    if 1, byte 1 of this entry is a leaf value.
 *              otherwise, it's a child node index.
 *     bit 1    ditto for byte 2.
 *     bit 2-7  unused. must be zero.
 *   byte 1: child node index or leaf value (for encoded bit 0)
 *   byte 2: child node index or leaf value (for encoded bit 1)
 *
 * note: byte 2 of the last entry is omitted if the count of the
 * corresponding node is 0. it's safe because the decoder logic
 * should never access it.
 */

void
huff_table(const struct hufftree *tree, uint8_t *out, size_t *lenp)
{
        uint8_t *outp = out;
        const struct hnode *n = root_node(tree);
        if (n->count == 0) {
                goto done;
        }
        const struct hnode *nodes[255];
        unsigned int prod = 0;
        unsigned int cons = 0;
        nodes[prod++] = n;
        while (cons < prod) {
                // printf("cons %02x prod %02x\n", cons, prod);
                n = nodes[cons++];
                assert(!is_leaf(tree, n));
                uint8_t v[3];
                uint8_t flags = 0;
                unsigned int i;
                size_t entry_size = 3;
                for (i = 0; i < 2; i++) {
                        struct hnode *cn = n->u.inner.children[i];
                        if (is_leaf(tree, cn)) {
                                flags |= 1 << i;
                                v[i + 1] = leaf_value(tree, cn);
                        } else if (cn->count == 0) {
                                assert(i == 1);
                                assert(cons == prod);
                                entry_size = 2;
                        } else {
                                v[i + 1] = prod;
                                assert(prod < 255);
                                nodes[prod++] = cn;
                        }
                }
                v[0] = flags;
#if 0
                printf("%02x: %02x %02x %02x\n", cons - 1, (unsigned int)v[0],
                       (unsigned int)v[1], (unsigned int)v[2]);
#endif
                memcpy(outp, v, entry_size);
                outp += entry_size;
        }
done:
        assert(outp - out <= HUFF_TABLE_SIZE_MAX);
        *lenp = outp - out;
}
