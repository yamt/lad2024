#include "rule.h"

struct size {
        int xmin;
        int ymin;
        int xmax;
        int ymax;
};

#define map_size (map_height * map_width)

void measure_size(const map_t map, struct size *size);
void map_copy(map_t dst, const map_t src);
void map_fill(map_t map, uint8_t objidx);
