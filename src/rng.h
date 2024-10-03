#include <stdint.h>

struct rng {
        uint32_t state[16];
        unsigned int i;
};

void rng_init(struct rng *rng, uint64_t seed);
uint32_t rng_rand_u32(struct rng *rng);

int rng_rand(struct rng *rng, int min, int max);
