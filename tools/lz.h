#if !defined(_LAD_LZ_H_)
#define _LAD_LZ_H_

#include <stdint.h>

#include "lz_param.h"

typedef void (*output_literal_t)(void *ctx, uint8_t ch);
typedef void (*output_match_t)(void *ctx, woff_t dist, woff_t len);

#define LZ_ENCODE_BUF_SIZE (MATCH_DISTANCE_MAX + MATCH_LEN_MAX)

struct lz_encode_state {
        /* output callbacks */
        output_literal_t out_literal;
        output_match_t out_match;
        void *out_ctx;

        /* internal state */
        woff_t bufstart;
        woff_t curoff;     /* relative to bufstart */
        woff_t valid_size; /* relative to bufstart */
        uint8_t buf[LZ_ENCODE_BUF_SIZE];
};

void lz_encode_init(struct lz_encode_state *state);
void lz_encode(struct lz_encode_state *state, const void *p, size_t len);
void lz_encode_flush(struct lz_encode_state *state);

#endif /* !defined(_LAD_LZ_H_) */
