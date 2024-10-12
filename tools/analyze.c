#include "analyze.h"
#include "defs.h"
#include "maputil.h"
#include "rule.h"

void
visit(const map_t map, const map_t movable, loc_t loc, map_t reachable)
{
        if (reachable[loc] != UNREACHABLE) {
                return;
        }
        uint8_t objidx = map[loc];
        if (objidx != _ && movable[loc] == UNMOVABLE) {
                reachable[loc] = REACHABLE_WALL;
                return;
        }
        reachable[loc] = REACHABLE;
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
update_reachable_from(const map_t map, const map_t movable, loc_t loc,
                      map_t reachable)
{
        visit(map, movable, loc, reachable);
}

void
calc_reachable_from(const map_t map, const map_t movable, loc_t loc,
                    map_t reachable)
{
        map_fill(reachable, UNREACHABLE);
        update_reachable_from(map, movable, loc, reachable);
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

void
calc_reachable_from_any_A(const map_t map, const map_t movable,
                          map_t reachable)
{
        /* XXX cheaper to take stage meta */
        map_fill(reachable, UNREACHABLE);
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == A) {
                        update_reachable_from(map, movable, loc, reachable);
                }
        }
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
                                 * from UNMOVABLE to MOVABLE.
                                 */
                                for (dir = 0; dir < 4; dir++) {
                                        loc_t d = dirs[dir].loc_diff;
                                        loc_t nloc = loc + d;
                                        if (!in_map(nloc)) {
                                                continue;
                                        }
                                        if (movable[nloc] == UNMOVABLE) {
                                                /*
                                                 * need to investigate again
                                                 */
                                                movable[nloc] = UNVISITED;
                                                more = true;
                                        }
                                }
                        } else {
                                movable[loc] = UNMOVABLE;
                        }
                }
        } while (more);
}

/*
 * _W___
 * W____
 * WX___ X is SUR(LEFT).  it can't be collected with LIGHT(RIGHT).
 * W____
 * _WW__
 *
 * W____
 * WX___ X is SUR(LEFT)|SUR(DOWN).  you need LIGHT(LEFT) or LIGHT(DOWN).
 * _WW__ this is an easy case as the X is UNMOVABLE.
 *
 * note: if X is SUR(dir), it can't be collected with LIGHT(opposite(dir)).
 * ie. you need a light with other dir in the map.
 */

#define SUR(dir) DIRBIT(dir)
#define DIRBIT(dir) (1U << (dir))
#define LIGHT(dir) (L + (dir))
#define opposite(dir) (((dir) + 2) % 4)

bool
surrounded_dir(const map_t map, const map_t movable, loc_t loc, loc_t d1,
               loc_t d2)
{
        while (true) {
                loc_t nloc;

                nloc = loc + d1;
                if (in_map(nloc) && !occupied(map, movable, nloc)) {
                        break;
                }
                nloc = loc + d2;
                if (in_map(nloc) && !occupied(map, movable, nloc)) {
                        loc = nloc;
                        continue;
                }
                return true;
        }
        return false;
}

void
calc_surrounded(const map_t map, const map_t movable, map_t surrounded)
{
        map_fill(surrounded, 0);
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                /*
                 * for now, we only care X.
                 * can this be useful for other objects?
                 */
                if (objidx != X) {
                        continue;
                }
                enum diridx dir;
                /*
                 * XXX notyet; we don't mark X UNMOVABLE unless
                 * it's obviously uncollectable. maybe we shoud
                 * distinguish collectability from pushability.
                 */
#if 0
                if (movable[loc] == UNMOVABLE) {
                        /*
                         * the easy case.
                         */
                        for (dir = 0; dir < 4; dir++) {
                                loc_t nloc = loc + dirs[dir].loc_diff;
                                if (occupied(map, movable, nloc)) {
                                        surrounded[loc] |= SUR(dir);
                                }
                        }
                        continue;
                }
#endif
                for (dir = 0; dir < 4; dir++) {
                        loc_t d = dirs[dir].loc_diff;
                        loc_t d2 = dirs[(dir + 1) % 4].loc_diff;
                        if (!surrounded_dir(map, movable, loc, d, d2)) {
                                continue;
                        }
                        loc_t d3 = dirs[(dir + 3) % 4].loc_diff;
                        if (!surrounded_dir(map, movable, loc, d, d3)) {
                                continue;
                        }
                        surrounded[loc] |= SUR(dir);
                }
        }
}

bool
tsumi(const map_t map)
{
        map_t movable;
        calc_movable(map, movable);
        map_t reachable;
        calc_reachable_from_any_A(map, movable, reachable);
        map_t surrounded;
        calc_surrounded(map, movable, surrounded);
        unsigned int counts[END];
        count_objects(map, counts);

        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == X) {
                        /*
                         * note: calc_movable mark an X UNMOVABLE only when
                         * it's impossible to be collected.
                         */
                        if (movable[loc] == UNMOVABLE) {
                                return true;
                        }
                        if (reachable[loc] == UNREACHABLE) {
                                return true;
                        }
                        uint8_t sur = surrounded[loc];
                        if (sur != 0) {
                                enum diridx dir;
                                for (dir = 0; dir < 4; dir++) {
                                        if ((sur & SUR(dir)) != 0) {
                                                continue;
                                        }
                                        if (counts[LIGHT(opposite(dir))] ==
                                            0) {
                                                continue;
                                        }
                                        break;
                                }
                                if (dir >= 4) {
                                        return true;
                                }
                        }
                }
        }
        return false;
}
