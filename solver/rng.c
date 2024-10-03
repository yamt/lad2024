#include "rng.h"

void
rng_init(struct rng *rng, uint64_t seed)
{
        unsigned int i;
        for (i = 0; i < 16; i += 2) {
                rng->state[i] = (uint32_t)seed;
                rng->state[i + 1] = seed >> 32;

                /*
                 * simple LCG with parameters taken
                 * from posix drand48 family
                 */
                seed = seed * 0x5DEECE66 + 0xB;
        }
        rng->i = 0;
}

static unsigned int
add(unsigned int a, unsigned int b)
{
        return (a + b) % 16;
}

static uint32_t
lshiftxor(uint32_t a, unsigned int shift)
{
        return a ^ (a << shift);
}

static uint32_t
rshiftxor(uint32_t a, unsigned int shift)
{
        return a ^ (a >> shift);
}

/*
 * WELL512a
 *
 * http://www.iro.umontreal.ca/~lecuyer/myftp/papers/wellrng.pdf
 * https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear
 */
uint32_t
rng_rand_u32(struct rng *rng)
{
        uint32_t *st = rng->state;
        unsigned int i = rng->i;

        uint32_t a = st[i];
        uint32_t b = st[add(i, 13)];
        uint32_t c = st[add(i, 9)];

        uint32_t x = lshiftxor(a, 16) ^ lshiftxor(b, 15);
        uint32_t y = rshiftxor(c, 11);

        a = st[i] = x ^ y;
        uint32_t z = a ^ ((a << 5) & 0xda442d24);

        i = add(i, 15);
        a = st[i];
        st[i] = lshiftxor(a, 2) ^ lshiftxor(x, 18) ^ (y << 28) ^ z;
        rng->i = i;

        return st[i];
}

int
rng_rand(struct rng *rng, int min, int max)
{
        uint32_t v = rng_rand_u32(rng);
        return min + (int)(v % (uint32_t)(max - min + 1));
}
