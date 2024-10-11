#undef NDEBUG
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "rng.h"

int
main(void)
{
        struct rng rng;
        unsigned int i;
        uint32_t min;
        uint32_t max;
        uint32_t x;

        rng_init(&rng, 0);
        for (i = 0; i < 16; i++) {
                rng.state[i] = 0;
        }
        rng.state[15] = 1;
        min = UINT32_MAX;
        max = 0;
        for (i = 0; i < 9999; i++) {
                x = rng_rand_u32(&rng);
                if (min > x) {
                        min = x;
                }
                if (max < x) {
                        max = x;
                }
        }
        printf("x    %" PRIx32 "\n", x);
        printf("min  %" PRIx32 "\n", min);
        printf("max  %" PRIx32 "\n", max);
        /* https://github.com/sergiud/random/blob/cdd9562656e8e3739c7ee6a468e25c2cf60b64ee/tests/well/main.cpp#L123
         */
        assert(x == 0x4df08652);

        rng_init(&rng, 0);
        for (i = 0; i < 16; i++) {
                rng.state[i] = 1;
        }
        min = UINT32_MAX;
        max = 0;
        for (i = 0; i < 1000000000; i++) {
                x = rng_rand_u32(&rng);
                if (min > x) {
                        min = x;
                }
                if (max < x) {
                        max = x;
                }
        }
        printf("x    %" PRIx32 "\n", x);
        printf("min  %" PRIx32 "\n", min);
        printf("max  %" PRIx32 "\n", max);
        /* https://github.com/sergiud/random/blob/cdd9562656e8e3739c7ee6a468e25c2cf60b64ee/tests/well/main.cpp#L87C53-L87C63
         */
        assert(x == 0x2b3fe99e);
}
