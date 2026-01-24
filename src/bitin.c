/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "bitin.h"
#include "rans_param.h"

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
        /* within a byte, read MSB first. */
        uint8_t bit = (u8 >> (7 - in->bitoff)) & 1;
        in->bitoff++;
        if (in->bitoff == 8) {
                in->bitoff = 0;
                in->p++;
        }
        return bit;
}

uint16_t
bitin_get_bits(struct bitin *in, unsigned int nbits)
{
        RANS_ASSERT(in->bitoff < 8);
        RANS_ASSERT(nbits <= 16);
        unsigned int bitoff = in->bitoff;
        uint32_t u = 0;
        while (nbits > 0) {
                uint8_t mask = 0xff >> bitoff;
                u += *in->p & mask;
                if (bitoff + nbits < 8) {
                        bitoff += nbits;
                        break;
                }
                /* advance to the next byte */
                nbits -= 8 - bitoff;
                bitoff = 0;
                in->p++;
                u <<= 8;
        }
        in->bitoff = bitoff;
        RANS_ASSERT(u <= 0xffffff);
        u >>= 8 - bitoff;
        RANS_ASSERT(u <= 0xffff);
        return (uint16_t)u;
}
