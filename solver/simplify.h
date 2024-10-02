#include "rule.h"

void simplify(map_t map);
void align_to_top_left(map_t map);

bool calc_reachable_from_A(const map_t map, const map_t movable,
                           map_t reachable);
void calc_movable(const map_t map, map_t movable);
