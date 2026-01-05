#if !defined(_LAD_BITIN_H_)
#define _LAD_BITIN_H_

#include <stdint.h>

struct bitin {
        const uint8_t *p;
        unsigned int bitoff;
};

void bitin_init(struct bitin *in, const uint8_t *p);
uint8_t bitin_get_bit(struct bitin *in);

#endif /* !defined(_LAD_BITIN_H_) */
