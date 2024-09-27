#include <string.h>

#include "defs.h"
#include "dump.h"
#include "maputil.h"
#include "rule.h"
#include "simplify.h"

#define UNVISITED 0
#define VISITED 1
#define MOVABLE 2
#define UNMOVABLE 3

void
visit(const map_t map, const map_t movable, loc_t loc, map_t reachable)
{
        uint8_t objidx = map[loc];
        if (objidx != _ && movable[loc] == UNMOVABLE) {
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
                visit(map, movable, nloc, reachable);
        }
}

void
calc_reachable_from(const map_t map, const map_t movable, loc_t loc,
                    map_t reachable)
{
        map_fill(reachable, 0);
        visit(map, movable, loc, reachable);
}

bool
calc_reachable_from_A(const map_t map, const map_t movable, map_t reachable)
{
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == A) {
                        break;
                }
        }
        if (loc == map_size) {
                return true;
        }
        calc_reachable_from(map, movable, loc, reachable);
        return false;
}

bool
simplify_unreachable(map_t map, const map_t movable)
{
        map_t reachable;
        if (calc_reachable_from_A(map, movable, reachable)) {
                return false;
        }
        bool modified = false;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx != W && !reachable[loc]) {
                        map[loc] = W;
                        modified = true;
                }
        }
        return modified;
}

bool
is_simple_movable_object(uint8_t objidx)
{
        /*
         * players can move by themselves.
         * bomb can be collected.
         */
        return can_push(objidx) && !is_player(objidx) && objidx != X;
}

bool
occupied(const map_t map, const map_t movable, loc_t loc)
{
        if (!in_map(loc)) {
                return true;
        }
        uint8_t objidx = map[loc];
        if (objidx == W) {
                return true;
        }
        if (is_simple_movable_object(objidx) && movable[loc] != MOVABLE) {
                return true;
        }
        return false;
}

void
calc_movable(const map_t map, map_t movable)
{
        map_fill(movable, UNVISITED);

        bool more;
        do {
                more = false;
                loc_t loc;
                for (loc = 0; loc < map_size; loc++) {
                        if (movable[loc] != 0) {
                                continue;
                        }
                        uint8_t objidx = map[loc];
                        if (objidx == W) {
                                movable[loc] = UNMOVABLE;
                                more = true;
                                continue;
                        }
                        if (!is_simple_movable_object(objidx)) {
                                movable[loc] = VISITED;
                                more = true;
                                continue;
                        }
                        bool might_move = false;
                        enum diridx dir;
                        for (dir = 0; dir < 2; dir++) {
                                loc_t d = dirs[dir].loc_diff;
                                if (!occupied(map, movable, loc + d) &&
                                    !occupied(map, movable, loc - d)) {
                                        might_move = true;
                                        break;
                                }
                        }
                        if (might_move) {
                                movable[loc] = MOVABLE;
                                /*
                                 * note: mark neighbors unvisited.
                                 * this might effectively degrade them
                                 * from 3 to 2.
                                 */
                                for (dir = 0; dir < 4; dir++) {
                                        loc_t d = dirs[dir].loc_diff;
                                        loc_t nloc = loc + d;
                                        if (!in_map(nloc)) {
                                                continue;
                                        }
                                        if (movable[nloc] == UNMOVABLE) {
                                                movable[nloc] = 0;
                                                more = true;
                                        }
                                }
                        } else {
                                movable[loc] = UNMOVABLE;
                        }
                }
        } while (more);
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
                if (movable[loc] != UNMOVABLE) {
                        continue;
                }
                if (is_light(objidx)) {
                        /*
                         * detect trivial cases like:
                         *
                         * RL
                         * UU
                         */
                        loc_t nloc = loc + dirs[light_dir(objidx)].loc_diff;
                        if (!in_map(nloc) || (movable[nloc] == UNMOVABLE &&
                                              block_beam(map[nloc]))) {
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
        calc_movable(map, movable);

        turn_unmovable_to_W(map, movable);
        simplify_unreachable(map, movable);

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
