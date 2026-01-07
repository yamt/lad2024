#include <stdint.h>

#include "lz_param.h"

#define LZ_DECODE_BUF_SIZE (MATCH_DISTANCE_MAX + MATCH_LEN_MAX)

struct lz_decode_state {
    woff_t readidx;
    woff_t endidx;
    uint8_t buf[LZ_DECODE_BUF_SIZE];
};

void lz_decode_init(struct lz_decode_state *s);
void lz_apply_match(uint8_t *cur, woff_t len, woff_t dist);
