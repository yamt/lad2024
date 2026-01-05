/*
 * a small huffman decoder.
 *
 * see the comments in the encoder for the encoding details.
 */

#include <stdint.h>

#include "huff_types.h"

struct bitin;
huff_sym_t huff_decode_sym(struct bitin *in, const uint8_t *table);
