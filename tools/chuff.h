/*
 * context-dependent huffman encoding
 *
 * use a one-byte context to select hufftree to use.
 */

#include "huff.h"

#define CHUFF_NTABLES 256
struct chuff {
        uint8_t context;  /* current index in trees[]. 0..CHUFF_NTABLES-1 */
        uint16_t ntables; /* used elements in trees[]. 0..CHUFF_NTABLES */
        struct hufftree trees[CHUFF_NTABLES];
};

void chuff_init(struct chuff *ch);
void chuff_update(struct chuff *ch, const uint8_t *p, size_t len);
void chuff_build(struct chuff *ch);
void chuff_encode(struct chuff *ch, const uint8_t *p, size_t len,
                  struct bitbuf *os);

#define CHUFF_TABLE_SIZE_MAX (CHUFF_NTABLES * HUFF_TABLE_SIZE_MAX)
void chuff_table(const struct chuff *ch, uint8_t *out,
                 uint8_t *outsp[CHUFF_NTABLES], size_t lensp[CHUFF_NTABLES]);
