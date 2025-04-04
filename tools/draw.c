#include <assert.h>

#include "bb.h"
#include "defs.h"
#include "draw.h"
#include "maputil.h"
#include "rng.h"

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

#if 0
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
#endif

bool
random_place_objs_in_bb_with_freq(struct rng *rng, map_t map,
                                  const struct bb *bb, const struct freq *freq,
                                  unsigned int nfreq)
{

        unsigned int total = 0;
        unsigned int i;
        for (i = 0; i < nfreq; i++) {
                total += freq[i].freq;
        }

        int y;
        for (y = 0; y < bb->h; y++) {
                int x;
                for (x = 0; x < bb->w; x++) {
                        loc_t loc = genloc(bb->x + x, bb->y + y);
                        if (map[loc] != _) {
                                continue;
                        }
                        uint8_t objidx = END;
                        unsigned int r = rng_rand(rng, 0, total - 1);
                        for (i = 0; i < nfreq; i++) {
                                const struct freq *f = &freq[i];
                                if (r < f->freq) {
                                        objidx = f->objidx;
                                        break;
                                }
                                r -= f->freq;
                        }
                        assert(i < nfreq);
                        assert(objidx != END);
                        map[loc] = objidx;
                }
        }
        return false;
}

bool
random_place_objs_in_bb(struct rng *rng, map_t map, const struct bb *bb)
{
        struct freq freq[] = {
                {_, 16}, {X, 5}, {B, 2}, {U, 2},
                {R, 2},  {D, 2}, {L, 2}, {W, 1},
        };
        unsigned int nfreq = sizeof(freq) / sizeof(freq[0]);
        return random_place_objs_in_bb_with_freq(rng, map, bb, freq, nfreq);
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

void
rect(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx)
{
        int x;
        int y;
        for (y = 0; y < rh; y++) {
                for (x = 0; x < rw; x++) {
                        map[genloc(rx + x, ry + y)] = objidx;
                }
        }
}

void
box(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx)
{
        rect(map, rx, ry, rw, 1, objidx);
        rect(map, rx, ry, 1, rh, objidx);
        rect(map, rx, ry + rh - 1, rw, 1, objidx);
        rect(map, rx + rw - 1, ry, 1, rh, objidx);
}

void
circle(map_t map, int ox, int oy, int r, uint8_t objidx)
{
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                int x = loc_x(loc);
                int y = loc_y(loc);
                int dx = x - ox;
                int dy = y - oy;
                if (dx * dx + dy * dy <= r * r) {
                        map[loc] = objidx;
                }
        }
}

void
hmirror(map_t map, int w, int h)
{
        int x;
        int y;
        for (y = 0; y < h; y++) {
                for (x = 0; x < w / 2; x++) {
                        map[genloc(x, y)] = map[genloc(w - x - 1, y)];
                }
        }
}

void
vmirror(map_t map, int w, int h)
{
        int x;
        int y;
        for (y = 0; y < h / 2; y++) {
                for (x = 0; x < w; x++) {
                        map[genloc(x, y)] = map[genloc(x, h - y - 1)];
                }
        }
}
