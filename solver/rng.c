#include <stdlib.h>

#include "rng.h"

void
rng_init(struct rng *rng, int seed)
{
        srand(seed);
}

int
rng_rand(struct rng *rng, int min, int max)
{
        return min + (rand() % (max - min + 1));
}
