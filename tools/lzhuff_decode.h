#include <stdint.h>

#include "huff_types.h"

struct lz_decode_state;
struct bitin;

void lzhuff_decode_init(struct lz_decode_state *s);
huff_sym_t lzhuff_decode_sym(struct lz_decode_state *s, struct bitin *in,
                             const uint8_t *huff_lit, const uint8_t *huff_dist,
                             huff_sym_t match_base);
