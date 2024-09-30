#include <stdbool.h>

#include "bb.h"

bool
xy_in_bb(const struct bb *bb, int x, int y)
{
        if (x < bb->x) {
                return false;
        }
        if (y < bb->y) {
                return false;
        }
        if (x > bb->x + bb->w - 1) {
                return false;
        }
        if (y > bb->y + bb->h - 1) {
                return false;
        }
        return true;
}

bool
loc_in_bb(const struct bb *bb, loc_t loc)
{
        return xy_in_bb(bb, loc_x(loc), loc_y(loc));
}

