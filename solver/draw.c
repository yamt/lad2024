#include "defs.h"
#include "draw.h"
#include "rng.h"
#include "bb.h"

bool
simple_put(uint8_t *p, void *arg)
{
        if (*p == _) {
                uint8_t *op = arg;
                *p = *op;
                return false;
        }
        return true;
}

bool
random_place_obj_in_bb(struct rng *rng, map_t map, const struct bb *bb,
                       putter_t putter, void *arg)
{
        int tries = 32;
        do {
                int x = rng_rand(rng, bb->x, bb->x + bb->w - 2);
                int y = rng_rand(rng, bb->y, bb->y + bb->h - 2);
                loc_t loc = genloc(x, y);
                if (!putter(&map[loc], arg)) {
                        return false;
                }
        } while (tries--);
        return true;
}

bool
random_place_objs_in_bb(struct rng *rng, map_t map, const struct bb *bb)
{
        int n;
        struct obj {
                uint8_t objidx;
                int min;
                int max;
        } objs[] = {
                {X, 1, 5}, {B, 0, 10}, {U, 0, 6},
                {R, 0, 6}, {D, 0, 6},  {L, 0, 6},
        };
        int j;
        for (j = 0; j < sizeof(objs) / sizeof(objs[0]); j++) {
                int i;
                const struct obj *o = &objs[j];
                n = rng_rand(rng, o->min, o->max);
                for (i = 0; i < n; i++) {
                        if (random_place_obj_in_bb(rng, map, bb, simple_put,
                                                   (void *)&o->objidx)) {
                                return true;
                        }
                }
        }
        return false;
}

bool
replace_obj(uint8_t *p, void *arg)
{
        if (*p != _ && *p != W) {
                uint8_t *op = arg;
                *p = *op;
                return false;
        }
        return true;
}
