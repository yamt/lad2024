#include <assert.h>
#include <stddef.h>

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
        /*
         * NOTE: this function is also used by calc_movable itself.
         */
        if (is_simple_movable_object(objidx) && is_UNMOVABLE(movable[loc])) {
                return true;
        }
        return false;
}

void
visit(const map_t map, const map_t movable, const map_t beam, loc_t loc,
      map_t reachable)
{
        if (reachable[loc] != UNREACHABLE) {
                return;
        }
        if (occupied(map, movable, loc)) {
                reachable[loc] = REACHABLE_WALL;
                return;
        }
        reachable[loc] = REACHABLE;
        if (beam != NULL && !beam[loc]) {
                /*
                 * beam != NULL implies A.
                 * !beam[loc] means this location can't be beamed.
                 *
                 * here, this A can't move by itself. however, it
                 * might be able to be pushed by other A or P.
                 */
                enum diridx dir;
                for (dir = 0; dir < 2; dir++) {
                        loc_t d = dirs[dir].loc_diff;
                        if (occupied(map, movable, loc + d) ||
                            occupied(map, movable, loc - d)) {
                                continue;
                        }
                        loc_t nloc;
                        nloc = loc + d;
                        if (in_map(nloc)) {
                                visit(map, movable, beam, nloc, reachable);
                        }
                        nloc = loc - d;
                        if (in_map(nloc)) {
                                visit(map, movable, beam, nloc, reachable);
                        }
                }
                return;
        }
        enum diridx dir;
        for (dir = 0; dir < 4; dir++) {
                loc_t nloc = loc + dirs[dir].loc_diff;
                if (!in_map(nloc)) {
                        continue;
                }
                visit(map, movable, beam, nloc, reachable);
        }
}

void
update_reachable_from(const map_t map, const map_t movable, const map_t beam,
                      loc_t loc, map_t reachable)
{
        visit(map, movable, beam, loc, reachable);
}

void
calc_reachable_from(const map_t map, const map_t movable, loc_t loc,
                    map_t reachable)
{
        map_fill(reachable, UNREACHABLE);
        update_reachable_from(map, movable, NULL, loc, reachable);
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
                          const map_t beam, map_t reachable)
{
        /* XXX cheaper to take stage meta */
        map_fill(reachable, UNREACHABLE);
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == A) {
                        update_reachable_from(map, movable, beam, loc,
                                              reachable);
                }
        }
}

bool
set_movable_bits(map_t movable, loc_t loc, uint8_t new)
{
        uint8_t old = movable[loc];
        movable[loc] = (old & ~NEEDVISIT) | new;
        /*
         * note: when we are degrading from UNMOVABLE
         * to MOVABLE, mark neighbors unvisited.
         * this might effectively degrade them
         * from UNMOVABLE to MOVABLE too.
         */
        if (!is_UNMOVABLE(old)) {
                return false;
        }
        bool more = false;
        enum diridx dir;
        for (dir = 0; dir < 4; dir++) {
                loc_t d = dirs[dir].loc_diff;
                loc_t nloc = loc + d;
                if (!in_map(nloc)) {
                        continue;
                }
                /*
                 * investigate again
                 * unless both bits are already set
                 */
                if ((~movable[nloc] & (PUSHABLE | COLLECTABLE)) != 0) {
                        movable[nloc] |= NEEDVISIT;
                        more = true;
                }
        }
        return more;
}

void
update_movable(const map_t map, bool do_collect, map_t movable)
{
        bool more;
        do {
                more = false;
                loc_t loc;
                for (loc = 0; loc < map_size; loc++) {
                        if ((movable[loc] & NEEDVISIT) == 0) {
                                continue;
                        }
                        uint8_t objidx = map[loc];
                        if (objidx == W) {
                                movable[loc] &= ~NEEDVISIT;
                                continue;
                        }
                        if (!is_simple_movable_object(objidx)) {
                                movable[loc] &= ~NEEDVISIT;
                                continue;
                        }
                        /*
                         * note: false positive is ok for both of
                         * might_push/might_collect
                         */
                        bool might_push = false;
                        bool might_collect = false;
                        enum diridx dir;
                        if (do_collect && objidx == X) {
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

                                /*
                                 * REVISIT: the collectability logic here
                                 * is very naive.
                                 * possibly_collectable() has a bit more
                                 * accurate logic, which is based on the
                                 * movability we are calculating here.
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
                                uint8_t new = 0;
                                if (might_push) {
                                        new |= PUSHABLE;
                                }
                                if (might_collect) {
                                        new |= COLLECTABLE;
                                }
                                more |= set_movable_bits(movable, loc, new);
                        } else {
                                movable[loc] &= ~NEEDVISIT;
                        }
                }
        } while (more);
}

void
calc_movable(const map_t map, bool do_collect, map_t movable)
{
        map_fill(movable, NEEDVISIT);
        update_movable(map, do_collect, movable);
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
#if 0 /* broken; see the XXX comment in calc_movable */
                bool easy = (movable[loc] & PUSHABLE) == 0;
#else
                bool easy = false;
#endif
                for (dir = 0; dir < 4; dir++) {
                        loc_t d = dirs[dir].loc_diff;
                        loc_t nloc = loc + d;
                        /*
                         * XXX this occupied should check the case of light
                         * as surrounded_dir does. maybe this should be
                         * moved to surrounded_dir.
                         */
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
                        continue;
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

bool
permanently_blocked(const map_t map, const map_t movable, loc_t loc)
{
        /*
         * detect trivial cases like:
         *
         * RL
         * UU
         */
        uint8_t objidx = map[loc];
        assert(is_light(objidx));
        loc_t nloc = loc + dirs[light_dir(objidx)].loc_diff;
        if (!in_map(nloc) ||
            (is_UNMOVABLE(movable[nloc]) && block_beam(map[nloc]))) {
                return true;
        }
        return false;
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
                        if (block_beam(map[nloc]) &&
                            is_UNMOVABLE(movable[nloc])) {
                                break;
                        }
                        beam[nloc] = 1;
                        nloc += d;
                }
        }
}

bool
possibly_collectable(const map_t map, const map_t movable, loc_t X_loc,
                     const map_t any_A_reachable, const map_t possible_beam[4])
{
        assert(map[X_loc] == X);
        map_t bomb_reachable;
        map_fill(bomb_reachable, UNREACHABLE);
        update_reachable_by_push_from(map, movable, X_loc, bomb_reachable);
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
                if (!any_A_reachable[loc]) {
                        continue;
                }
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        if (!possible_beam[dir][loc]) {
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

void
calc_possible_beam(const map_t map, const map_t movable,
                   map_t possible_beam[4])
{
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
        for (dir = 0; dir < 4; dir++) {
                map_fill(possible_beam[dir], 0);
                update_possible_beam(map, movable, light_reachable[dir], dir,
                                     possible_beam[dir]);
        }
}

void
calc_reachable_from_any_A_with_possible_beam(const map_t map,
                                             const map_t movable,
                                             const map_t possible_beam[4],
                                             map_t any_A_reachable)
{
        map_t possible_beam_any;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t v = 0;
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        v |= possible_beam[dir][loc];
                }
                possible_beam_any[loc] = v;
        }
        calc_reachable_from_any_A(map, movable, possible_beam_any,
                                  any_A_reachable);
}

/*
 * returns true if the map is impossible to clear. ("tsumi")
 *
 * false negative is ok.
 * false positive is not ok. (considered a bug)
 */
bool
tsumi(const map_t map)
{
        map_t movable;
        calc_movable(map, true, movable);
#if 0
        map_t surrounded;
        calc_surrounded(map, movable, surrounded);
        unsigned int counts[END];
        count_objects(map, counts);
#endif
        map_t possible_beam[4];
        calc_possible_beam(map, movable, possible_beam);
        map_t any_A_reachable;
        calc_reachable_from_any_A_with_possible_beam(
                map, movable, possible_beam, any_A_reachable);

        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == X) {
                        if ((movable[loc] & COLLECTABLE) == 0) {
                                return true;
                        }
#if 0
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
#endif
                        if (!possibly_collectable(map, movable, loc,
                                                  any_A_reachable,
                                                  possible_beam)) {
                                return true;
                        }
                }
        }
        return false;
}
