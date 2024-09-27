#include <string.h>

#include "defs.h"
#include "maputil.h"
#include "rule.h"
#include "simplify.h"

void
visit(map_t map, loc_t loc, map_t reachable)
{
        uint8_t objidx = map[loc];
        if (objidx == W) {
                return;
        }
        if (reachable[loc]) {
                return;
        }
        reachable[loc] = 1;
        enum diridx dir;
        for (dir = 0; dir < 4; dir++) {
                loc_t nloc = loc + dirs[dir].loc_diff;
                if (!in_map(nloc)) {
                        continue;
                }
                visit(map, nloc, reachable);
        }
}

void
calc_reachable_from(map_t map, loc_t loc, map_t reachable)
{
        map_fill(reachable, 0);
        visit(map, loc, reachable);
}

bool
calc_reachable_from_A(map_t map, map_t reachable)
{
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == A) {
                        break;
                }
        }
        if (loc == map_width * map_height) {
                return true;
        }
        calc_reachable_from(map, loc, reachable);
        return false;
}

bool
simplify_unreachable(map_t map)
{
        map_t reachable;
        if (calc_reachable_from_A(map, reachable)) {
                return false;
        }
        bool modified = false;
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                uint8_t objidx = map[loc];
                if (objidx != W && !reachable[loc]) {
                        map[loc] = W;
                        modified = true;
                }
        }
        return modified;
}

/* turn unmovable objects to W */
bool
simplify_unmovable(map_t map)
{
        map_t unmovable;
        map_fill(unmovable, 0);

        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == W) {
                        unmovable[loc] = 1;
                }
        }

        bool modified = false;
        bool more;
        do {
                more = false;
                for (loc = 0; loc < map_width * map_height; loc++) {
                        if (unmovable[loc]) {
                                continue;
                        }
                        uint8_t objidx = map[loc];
                        if (can_push(objidx) && !is_player(objidx) &&
                            objidx != X) {
                                enum diridx dir;
                                for (dir = 0; dir < 4; dir++) {
                                        loc_t nloc;
                                        nloc = loc + dirs[dir].loc_diff;
                                        if (in_map(nloc) && !unmovable[nloc]) {
                                                continue;
                                        }
                                        nloc = loc +
                                               dirs[(dir + 1) % 4].loc_diff;
                                        if (in_map(nloc) && !unmovable[nloc]) {
                                                continue;
                                        }
                                        unmovable[loc] = 1;
                                        more = true;
                                        if (is_light(objidx)) {
                                                /*
                                                 * detect trivial cases like:
                                                 *
                                                 * RL
                                                 */
                                                nloc = loc +
                                                       dirs[light_dir(objidx)]
                                                               .loc_diff;
                                                if (!in_map(nloc) ||
                                                    block_beam(map[nloc])) {
                                                        map[loc] = W;
                                                        modified = true;
                                                }
                                        } else {
                                                map[loc] = W;
                                                modified = true;
                                        }
                                        break;
                                }
                        }
                }
        } while (more);
        return modified;
}

void
simplify(map_t map)
{
        simplify_unmovable(map);
        simplify_unreachable(map);

        /*
         * remove redundant W outside of the map
         * REVISIT: maybe it's simpler to unify with simplify_unreachable
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

        /* align to top-left */
        measure_size(map, &size);
        if (size.xmin > 0 || size.ymin > 0) {
                loc_t off = genloc(size.xmin, size.ymin);
                memmove(&map[0], &map[off], map_width * map_height - off);
                memset(&map[map_width * map_height - off], 0, off);
        }
}
