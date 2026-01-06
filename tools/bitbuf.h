/*
 * bit-stream output buffer
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct bitbuf {
        uint8_t *p;
        uint32_t buf;
        unsigned int bufoff;
        size_t datalen;
        size_t allocated;
};

void bitbuf_init(struct bitbuf *s);
void bitbuf_write(struct bitbuf *s, const uint8_t *bits, uint16_t nbits);
void bitbuf_flush(struct bitbuf *s);
void bitbuf_clear(struct bitbuf *s);
