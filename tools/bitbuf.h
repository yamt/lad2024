/*
 * bit-stream output buffer
 *
 * the format of the output bit stream: within a byte, MSB are first.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct bitbuf {
        /* small buffer to concat bits together */
        uint32_t buf;
        unsigned int bufoff;

        /* byte output. */
        uint8_t *p;
        size_t datalen;
        size_t allocated;
};

void bitbuf_init(struct bitbuf *s);
/* bitbuf_write: append the least significant "nbits" of "bits". */
void bitbuf_write(struct bitbuf *s, const uint8_t *bits, uint16_t nbits);
void bitbuf_flush(struct bitbuf *s);
void bitbuf_clear(struct bitbuf *s);
