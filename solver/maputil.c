#include <string.h>

#include "defs.h"
#include "maputil.h"

void
measure_size(const map_t map, struct size *size)
{
        size->xmin = map_width - 1;
        size->ymin = map_height - 1;
        size->xmax = 0;
        size->ymax = 0;
        int x;
        int y;
        for (y = 0; y < map_height; y++) {
                for (x = 0; x < map_width; x++) {
                        if (map[genloc(x, y)] != _) {
                                if (size->xmin > x) {
                                        size->xmin = x;
                                }
                                if (size->ymin > y) {
                                        size->ymin = y;
                                }
                                if (size->xmax < x) {
                                        size->xmax = x;
                                }
                                if (size->ymax < y) {
                                        size->ymax = y;
                                }
                        }
                }
        }
}

void
map_copy(map_t dst, const map_t src)
{
        memcpy(dst, src, map_size);
}

void
map_fill(map_t map, uint8_t objidx)
{
        memset(map, objidx, map_size);
}

void
count_objects(const map_t map, unsigned int count[END])
{
        memset(count, 0, sizeof(count[0]) * END);
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                count[objidx]++;
        }
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

bool
anyeq(const map_t map, int rx, int ry, int rw, int rh, uint8_t objidx)
{
        int x;
        int y;
        for (y = 0; y < rh; y++) {
                for (x = 0; x < rw; x++) {
                        if (map[genloc(rx + x, ry + y)] == objidx) {
                                return true;
                        }
                }
        }
        return false;
}
