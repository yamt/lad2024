#include <stdint.h>

struct bitbuf {
        uint32_t buf;
        unsigned int bufoff;
};

void bitbuf_init(struct bitbuf *s);
void bitbuf_write(struct bitbuf *s, uint8_t **outpp, uint16_t bits,
                  uint8_t nbits);
void bitbuf_flush(struct bitbuf *s, uint8_t **outpp);
