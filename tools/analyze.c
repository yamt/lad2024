#include <assert.h>

#include "analyze.h"
#include "defs.h"
#include "maputil.h"
#include "rule.h"

bool
is_simple_movable_object(uint8_t objidx)
{
        /*
         * players can move by themselves.
         */
        return can_push(objidx) && !is_player(objidx);
}

/*
 * return if the specified location is permanently occupied.
 *
 * false-positive is not ok
 * false-negative is ok
 */
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
        if (is_simple_movable_object(objidx) && !is_MOVABLE(movable[loc])) {
                return true;
        }
        return false;
}

void
visit(const map_t map, const map_t movable, loc_t loc, map_t reachable)
{
        if (reachable[loc] != UNREACHABLE) {
                return;
        }
        if (occupied(map, movable, loc)) {
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

void
calc_movable(const map_t map, map_t movable)
{
        map_fill(movable, UNVISITED);

        bool more;
        do {
                more = false;
                loc_t loc;
                for (loc = 0; loc < map_size; loc++) {
                        if ((movable[loc]) != UNVISITED) {
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
                        bool might_push = false;
                        bool might_collect = false;
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
                                                might_collect = true;
                                                break;
                                        }
                                }
                        }
                        for (dir = 0; dir < 2; dir++) {
                                loc_t d = dirs[dir].loc_diff;
                                if (!occupied(map, movable, loc + d) &&
                                    !occupied(map, movable, loc - d)) {
                                        might_push = true;
                                        break;
                                }
                        }
                        if (might_push || might_collect) {
                                movable[loc] = MOVABLE;
                                if (might_push) {
                                        movable[loc] |= PUSHABLE;
                                }
                                if (might_collect) {
                                        movable[loc] |= COLLECTABLE;
                                }
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
                                        if (is_UNMOVABLE(movable[nloc])) {
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
               loc_t d2, bool easy)
{
        while (true) {
                loc_t nloc2 = loc + d2;
                if (occupied(map, movable, nloc2)) {
                        return true;
                }
                loc_t nloc1 = nloc2 + d1;
                if (in_map(nloc1)) {
                        uint8_t objidx = map[nloc1];
                        if (is_light(objidx) &&
                            dirs[opposite(light_dir(objidx))].loc_diff == d1) {
                                /*
                                 * eg. (d1 == LEFT, d2 == UP)
                                 *
                                 * _WW
                                 * WR_
                                 * _WX
                                 */
                                break;
                        }
                }
                if (!occupied(map, movable, nloc1)) {
                        break;
                }
                if (easy) {
                        return true;
                }
                loc = nloc2;
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
                bool easy = (movable[loc] & PUSHABLE) == 0;
                for (dir = 0; dir < 4; dir++) {
                        loc_t d = dirs[dir].loc_diff;
                        loc_t nloc = loc + d;
                        if (!occupied(map, movable, nloc)) {
                                continue;
                        }
                        loc_t d2 = dirs[(dir + 1) % 4].loc_diff;
                        if (!surrounded_dir(map, movable, loc, d, d2, easy)) {
                                continue;
                        }
                        loc_t d3 = dirs[(dir + 3) % 4].loc_diff;
                        if (!surrounded_dir(map, movable, loc, d, d3, easy)) {
                                continue;
                        }
                        surrounded[loc] |= SUR(dir);
                }
        }
}

void
visit_push(const map_t map, const map_t movable, loc_t loc, map_t reachable)
{
        if (reachable[loc] != UNREACHABLE) {
                return;
        }
        reachable[loc] = REACHABLE;
        enum diridx dir;
        for (dir = 0; dir < 2; dir++) {
                loc_t d = dirs[dir].loc_diff;
                if (occupied(map, movable, loc + d) ||
                    occupied(map, movable, loc - d)) {
                        return;
                }
                loc_t nloc;
                nloc = loc + d;
                if (in_map(nloc)) {
                        visit_push(map, movable, nloc, reachable);
                }
                nloc = loc - d;
                if (in_map(nloc)) {
                        visit_push(map, movable, nloc, reachable);
                }
        }
}

void
update_reachable_by_push_from(const map_t map, const map_t movable, loc_t loc,
                              map_t reachable)
{
        visit_push(map, movable, loc, reachable);
}

void
update_possible_beam(const map_t map, const map_t movable,
                     const map_t light_reachable, enum diridx dir, map_t beam)
{
        loc_t d = dirs[dir].loc_diff;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                if (!light_reachable[loc]) {
                        continue;
                }
                loc_t nloc = loc + d;
                while (true) {
                        if (beam[nloc]) {
                                break;
                        }
                        if (!in_map(nloc)) {
                                break;
                        }
                        if (is_UNMOVABLE(movable[nloc])) {
                                assert(block_beam(map[nloc]));
                                break;
                        }
                        beam[nloc] = 1;
                        nloc += d;
                }
        }
}

bool
possibly_collectable(const map_t bomb_reachable, const map_t possible_beam[4])
{
        /*
         * there are only a few patterns where
         * a bomb is collectable:
         *
         * ___
         * _X_
         * _A_
         * ___
         * _U_
         *
         * ____
         * _AX_
         * ____
         * _U__
         *
         * that is,
         * possible_beam[dir][A_loc] &&
         * (bomb_reachable[A_loc + dir_loc_diff(dir)] ||
         *  bomb_reachable[A_loc + dir_loc_diff(dir + 1)] ||
         *  bomb_reachable[A_loc + dir_loc_diff(dir - 1)])
         */
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        if (!possible_beam[loc]) {
                                continue;
                        }
                        loc_t nloc;
                        nloc = loc + dir_loc_diff(dir);
                        if (in_map(nloc) && bomb_reachable[nloc]) {
                                return true;
                        }
                        nloc = loc + dir_loc_diff((dir + 1) % 4);
                        if (in_map(nloc) && bomb_reachable[nloc]) {
                                return true;
                        }
                        nloc = loc + dir_loc_diff((dir + 3) % 4);
                        if (in_map(nloc) && bomb_reachable[nloc]) {
                                return true;
                        }
                }
        }
        return false;
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

        map_t light_reachable[4];
        enum diridx dir;
        for (dir = 0; dir < 4; dir++) {
                map_fill(light_reachable[dir], UNREACHABLE);
        }
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (!is_light(objidx)) {
                        continue;
                }
                enum diridx dir = light_dir(objidx);
                update_reachable_by_push_from(map, movable, loc,
                                              light_reachable[dir]);
        }
        map_t possible_beam[4];
        for (dir = 0; dir < 4; dir++) {
                map_fill(possible_beam[dir], 0);
                update_possible_beam(map, movable, light_reachable[dir], dir,
                                     possible_beam[dir]);
        }

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
                        map_t bomb_reachable;
                        map_fill(bomb_reachable, UNREACHABLE);
                        update_reachable_by_push_from(map, movable, loc,
                                                      bomb_reachable);
                        if (!possibly_collectable(bomb_reachable,
                                                  possible_beam)) {
                                return true;
                        }
                }
        }
        return false;
}
