#include <stdbool.h>
#include <stdint.h>

#include "rule.h"

struct rng;
struct bb;

typedef bool (*putter_t)(uint8_t *p, void *arg);

bool simple_put(uint8_t *p, void *arg);
bool replace_obj(uint8_t *p, void *arg);

bool random_place_obj_in_bb(struct rng *rng, map_t map, const struct bb *bb,
                            putter_t putter, void *arg);
bool random_place_objs_in_bb(struct rng *rng, map_t map, const struct bb *bb);

struct freq {
        uint8_t objidx;
        unsigned int freq;
};

bool random_place_objs_in_bb_with_freq(struct rng *rng, map_t map,
                                       const struct bb *bb,
                                       const struct freq *freq,
                                       unsigned int nfreq);

void rect(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx);
void box(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx);
void circle(map_t map, int x, int y, int r, uint8_t objidx);

void hmirror(map_t map, int w, int h);
void vmirror(map_t map, int w, int h);
