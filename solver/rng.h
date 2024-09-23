struct rng {
        int dummy;
};

void rng_init(struct rng *rng, int seed);
int rng_rand(struct rng *rng, int min, int max);
