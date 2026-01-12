#include <stddef.h>
#include <stdint.h>

struct byteout {
        uint8_t *p;
        size_t actual;
        size_t allocated;
};

void byteout_init(struct byteout *bo);
void byteout_write(struct byteout *bo, uint8_t byte);
void byteout_clear(struct byteout *bo);
void rev_byteout_write(struct byteout *bo, uint8_t byte);
void *rev_byteout_ptr(const struct byteout *bo);
