#include "rule.h"

struct bb {
        int x;
        int y;
        int w;
        int h;
};

bool xy_in_bb(const struct bb *bb, int x, int y);
bool loc_in_bb(const struct bb *bb, loc_t loc);
