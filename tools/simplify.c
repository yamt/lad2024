#include <assert.h>
#include <string.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
#include "maputil.h"
#include "rule.h"
#include "simplify.h"

bool
turn_unreachable_to_W(map_t map, const map_t reachable)
{
        bool modified = false;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx != W && reachable[loc] == UNREACHABLE) {
                        map[loc] = W;
                        modified = true;
                }
        }
        return modified;
}

/* turn unmovable objects to W */
bool
turn_unmovable_to_W(map_t map, const map_t movable)
{

        /* turn unmovable objects to W */
        bool modified = false;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (!is_simple_movable_object(objidx)) {
                        continue;
                }
                if (!is_UNMOVABLE(movable[loc])) {
                        continue;
                }
                if (is_light(objidx)) {
                        if (permanently_blocked(map, movable, loc)) {
                                map[loc] = W;
                                modified = true;
                        }
                } else {
                        map[loc] = W;
                        modified = true;
                }
        }
        return modified;
}

void
simplify(map_t map)
{
        map_t movable;
        calc_movable(map, true, movable);
        turn_unmovable_to_W(map, movable);
        map_t reachable;
        if (!calc_reachable_from_A(map, movable, reachable)) {
                turn_unreachable_to_W(map, reachable);
        }

        /*
         * remove redundant W outside of the map
         * REVISIT: maybe it's simpler to unify with turn_unreachable_to_W
         */
        struct size size;
        measure_size(map, &size);
        map_t orig;
        map_copy(orig, map);
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                if (map[loc] != W) {
                        continue;
                }
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        loc_t nloc = loc + dirs[dir].loc_diff;
                        if (!in_map(nloc)) {
                                continue;
                        }
                        int nx = loc_x(nloc);
                        int ny = loc_y(nloc);
                        if (nx < size.xmin || nx > size.xmax ||
                            ny < size.ymin || ny > size.ymax) {
                                continue;
                        }
                        if (orig[nloc] == W) {
                                continue;
                        }
                        break;
                }
                if (dir == 4) {
                        map[loc] = _;
                }
        }
}

void
align_to_top_left(map_t map)
{
        /* align to top-left */
        struct size size;
        measure_size(map, &size);
        if (size.xmin > 0 || size.ymin > 0) {
                loc_t off = genloc(size.xmin, size.ymin);
                memmove(&map[0], &map[off], map_width * map_height - off);
                memset(&map[map_width * map_height - off], 0, off);
        }
}
