#include "analyze.h"
#include "defs.h"
#include "maputil.h"
#include "rule.h"

void
visit(const map_t map, const map_t movable, loc_t loc, map_t reachable)
{
        if (reachable[loc]) {
                return;
        }
        reachable[loc] = 1;
        uint8_t objidx = map[loc];
        if (objidx != _ && movable[loc] == UNMOVABLE) {
                return;
        }
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

/*
 * returns true if failed. (no A is found in the map)
 */
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
is_simple_movable_object(uint8_t objidx)
{
        /*
         * players can move by themselves.
         */
        return can_push(objidx) && !is_player(objidx);
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
                        if (objidx == X) {
                                /*
                                 * note: X is considerd UNMOVABLE only if
                                 * surrounded by UNMOVABLE objects
                                 *
                                 * eg.
                                 * _____
                                 * _BB__
                                 * _BXB_ all X and B are UNMOVABLE
                                 * __BB_
                                 * _____
                                 *
                                 * eg.
                                 * _____
                                 * _BB__
                                 * _BXB_ all X and B are MOVABLE
                                 * _BB__
                                 * _____
                                 */
                                for (dir = 0; dir < 4; dir++) {
                                        loc_t d = dirs[dir].loc_diff;
                                        if (!occupied(map, movable, loc + d)) {
                                                might_move = true;
                                                break;
                                        }
                                }
                        } else {
                                for (dir = 0; dir < 2; dir++) {
                                        loc_t d = dirs[dir].loc_diff;
                                        if (!occupied(map, movable, loc + d) &&
                                            !occupied(map, movable, loc - d)) {
                                                might_move = true;
                                                break;
                                        }
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
