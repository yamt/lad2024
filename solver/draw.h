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

void rect(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx);
