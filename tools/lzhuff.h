#include "huff.h"
#include "lz.h"

/*
 * combine lz and huffman encoding similarly to deflate
 */

struct bitbuf;

struct lzhuff {
        struct hufftree huff_lit;  /* literals, match length */
        struct hufftree huff_dist; /* distance */
        struct lz_encode_state lz;
        struct bitbuf *os;
        huff_sym_t match_base;
};

void lzhuff_init(struct lzhuff *lzh, huff_sym_t match_base);
void lzhuff_update(struct lzhuff *lzh, const void *p, size_t len);
void lzhuff_build(struct lzhuff *lzh);

void lzhuff_encode_init(struct lzhuff *lzh, struct bitbuf *os);
void lzhuff_encode(struct lzhuff *lzh, const void *p, size_t len);

/* flush update/encode */
void lzhuff_flush(struct lzhuff *lzh);
