#include "bitin.h"

void
bitin_init(struct bitin *in, const uint8_t *p)
{
        in->p = p;
        in->bitoff = 0;
}

uint8_t
bitin_get_bit(struct bitin *in)
{
        uint8_t u8 = *in->p;
        uint8_t bit = (u8 >> (7 - in->bitoff)) & 1;
        in->bitoff++;
        if (in->bitoff == 8) {
                in->bitoff = 0;
                in->p++;
        }
        return bit;
}
