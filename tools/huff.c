#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

void
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

int
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

void
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
                 * pick the two nodes with the smallest count.
                 */
                qsort(nodes, 256 - i, sizeof(*nodes), cmp_node);
                struct hnode *inner = &tree->nodes[256 + i];
                struct hnode *na = nodes[256 - 2 - i];
                struct hnode *nb = nodes[256 - 1 - i];
                /*
                 * build a new inner node to represent the picked two nodes.
                 */
                inner->count = na->count + nb->count;
                inner->u.inner.children[0] = na;
                inner->u.inner.children[1] = nb;
                nodes[256 - 2 - i] = inner;
                nodes[256 - 1 - i] = NULL; /* just in case */
        }
}

void
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

void
finish_tree(struct hufftree *tree)
{
        finish_node(tree, &tree->nodes[256 * 2 - 2], 0, 0);
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

void
huff_encode(const struct hufftree *tree, const uint8_t *p, size_t len,
            uint8_t *out, size_t *lenp)
{
        const uint8_t *cp = p;
        const uint8_t *ep = cp + len;
        uint8_t *outp = out;
        uint32_t buf = 0;
        unsigned int bufoff = 0;
        while (cp < ep) {
                uint8_t c = *cp++;
                const struct hnode *n = &tree->nodes[c];
                assert(n->count > 0);
                assert(is_leaf(tree, n));
                uint16_t bits = n->u.leaf.encoded_bits;
                uint8_t nbits = n->u.leaf.encoded_nbits;
                assert(nbits > 0);
                unsigned int shift = 32 - bufoff - nbits;
                buf |= (uint32_t)bits << shift;
                bufoff += nbits;
                while (bufoff >= 8) {
                        *outp++ = buf >> 24;
                        buf <<= 8;
                        bufoff -= 8;
                }
        }
        if (bufoff > 0) {
                assert(bufoff < 8);
                *outp++ = buf >> 24;
        }
        assert(outp - out <= len);
        *lenp = outp - out;
}

void
huff_table(const struct hufftree *tree, uint8_t *out, size_t *lenp)
{
        uint8_t *outp = out;
        const struct hnode *n = &tree->nodes[256 * 2 - 2];
        const struct hnode *nodes[255];
        unsigned int prod = 0;
        unsigned int cons = 0;
        nodes[prod++] = n;
        while (cons < prod) {
                // printf("cons %02x prod %02x\n", cons, prod);
                n = nodes[cons++];
                assert(!n->is_leaf);
                uint8_t v[3];
                uint8_t flags = 0;
                unsigned int i;
                for (i = 0; i < 2; i++) {
                        struct hnode *cn = n->u.inner.children[i];
                        if (is_leaf(tree, cn)) {
                                flags |= 1 << i;
                                v[i + 1] = leaf_value(tree, cn);
                        } else if (cn->count == 0) {
                                v[i + 1] = 0;
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
                memcpy(outp, v, 3);
                outp += 3;
        }
        assert(outp - out <= HUFF_TABLE_SIZE_MAX);
        *lenp = outp - out;
}
