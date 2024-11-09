#include <stdint.h>

struct huff_decode_context {
	const uint8_t *p;
    unsigned int bitoff;
};

void huff_decode_init(struct huff_decode_context *ctx, const uint8_t *p);
uint8_t huff_decode_byte(struct huff_decode_context *ctx, const uint8_t *table);
