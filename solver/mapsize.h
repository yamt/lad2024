#include "rule.h"

struct size {
        int xmin;
        int ymin;
        int xmax;
        int ymax;
};

void measure_size(const map_t map, struct size *size);
