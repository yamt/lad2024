#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hnode {
        bool is_leaf;
        size_t count;
        union {
                struct leaf {
                        uint8_t value;
                        uint8_t encoded_nbits;
                        uint16_t encoded_bits;
                } leaf;
                struct inner {
                        struct hnode *children[2];
                        uint8_t idx;
                } inner;
        } u;
};

struct hufftree {
        struct hnode nodes[256 * 2 - 1];
};

void huff_init(struct hufftree *tree);
void huff_update(struct hufftree *tree, const uint8_t *p, size_t len);
void huff_build(struct hufftree *tree);

void huff_encode(const struct hufftree *tree, const uint8_t *p, size_t len,
                 uint8_t *out, size_t *lenp);

#define HUFF_TABLE_SIZE_MAX (255 * 3)
void huff_table(const struct hufftree *tree, uint8_t *out, size_t *lenp);
