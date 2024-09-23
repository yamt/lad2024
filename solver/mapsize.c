#include "mapsize.h"
#include "defs.h"

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