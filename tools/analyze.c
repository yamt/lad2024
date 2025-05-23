#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
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
set_movable_bits(map_t movable, loc_t loc, uint8_t newbits)
{
        uint8_t old = movable[loc];
        movable[loc] = (old & ~NEEDVISIT) | newbits;
        /*
         * Note: PUSHABLE and COLLECTABLE_AS_LAST are mutally exclusive.
         */
        if ((newbits & COLLECTABLE_AS_LAST) != 0) {
                assert((movable[loc] & PUSHABLE) == 0);
        }
        if ((newbits & PUSHABLE) != 0 &&
            (movable[loc] & COLLECTABLE_AS_LAST) != 0) {
                /* turn COLLECTABLE_AS_LAST to COLLECTABLE */
                movable[loc] &= ~COLLECTABLE_AS_LAST;
                movable[loc] |= COLLECTABLE;
                newbits |= COLLECTABLE;
        }
        /*
         * note: when we are degrading from UNMOVABLE
         * to MOVABLE, mark neighbors unvisited.
         * this might effectively degrade them
         * from UNMOVABLE to MOVABLE too.
         */
        if (!is_UNMOVABLE(old)) {
                return false;
        }
        /*
         * COLLECTABLE_AS_LAST alone doesn't affect is_MOVABLE.
         */
        if ((newbits & ~COLLECTABLE_AS_LAST) == 0) {
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
                                uint8_t newbits = 0;
                                if (might_push) {
                                        newbits |= PUSHABLE;
                                }
                                if (might_collect) {
                                        newbits |= COLLECTABLE;
                                }
                                more |= set_movable_bits(movable, loc,
                                                         newbits);
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
                        /*
                         * mark it true even if blocked.
                         * it's for the convenience of the
                         * COLLECTABLE_AS_LAST check.
                         */
                        beam[nloc] = 1;
                        if (block_beam(map[nloc]) &&
                            is_UNMOVABLE(movable[nloc])) {
                                break;
                        }
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
                        if (block_beam(map[loc]) &&
                            is_UNMOVABLE(movable[loc])) {
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
calc_possible_beam_any(const map_t possible_beam[4], map_t possible_beam_any)
{
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t v = 0;
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        v |= possible_beam[dir][loc];
                }
                possible_beam_any[loc] = v;
        }
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
        bool verbose = false;
        if (verbose) {
                printf("tsumi\n");
        }
        map_t movable;
        calc_movable(map, false, movable);
        unsigned int counts[END];
        count_objects(map, counts);
        map_t possible_beam[4];
retry:
        calc_possible_beam(map, movable, possible_beam);
        map_t possible_beam_any;
        calc_possible_beam_any(possible_beam, possible_beam_any);
        map_t any_A_reachable;
        calc_reachable_from_any_A(map, movable, possible_beam_any,
                                  any_A_reachable);

        if (verbose) {
                printf("map\n");
                dump_map(map);
                printf("movable\n");
                dump_map_raw(movable);
                printf("possible_beam_any\n");
                dump_map_raw(possible_beam_any);
                printf("any_A_reachable\n");
                dump_map_raw(any_A_reachable);
        }

        unsigned int last_X = 0;
        bool all_collectable = true;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                uint8_t objidx = map[loc];
                if (objidx == X) {
                        if (verbose) {
                                printf("checking X at [%u:%u] %02x\n",
                                       loc_x(loc), loc_y(loc), movable[loc]);
                        }
                        if ((movable[loc] & COLLECTABLE) != 0) {
                                assert((movable[loc] & COLLECTABLE_AS_LAST) ==
                                       0);
                                continue;
                        }
                        if ((movable[loc] & COLLECTABLE_AS_LAST) != 0) {
                                /*
                                 * check if "as last" condition is
                                 * still met.
                                 */
                                if ((movable[loc] & PUSHABLE) != 0 ||
                                    possible_beam_any[loc]) {
                                        /*
                                         * otherwise, turn it to COLLECTABLE.
                                         */
                                        movable[loc] &= ~COLLECTABLE_AS_LAST;
                                        goto set_collectable;
                                }
                                /*
                                 * because the location of this X
                                 * is not possibly beamed, the A
                                 * collected the X will not be able
                                 * to move by itself anymore.
                                 *
                                 * because this X is not PUSHABLE,
                                 * the A will not be pushable either.
                                 *
                                 * thus, the A will be unmovable after
                                 * collecting this X. that is, this X
                                 * needs to be collected by the last
                                 * move of the A.
                                 *
                                 * if the number of such Xs are larger
                                 * than the number of As in the map,
                                 * the map is not solvable.
                                 */
update_last_X:
                                if (verbose) {
                                        printf("last_X\n");
                                }
                                last_X++;
                                continue;
                        }
                        if (possibly_collectable(map, movable, loc,
                                                 any_A_reachable,
                                                 possible_beam)) {
                                uint8_t bit = COLLECTABLE_AS_LAST;
                                if ((movable[loc] & PUSHABLE) != 0 ||
                                    possible_beam_any[loc]) {
set_collectable:
                                        bit = COLLECTABLE;
                                }
                                if (set_movable_bits(movable, loc, bit)) {
                                        /*
                                         * propagate the change to neighbours
                                         *
                                         * COLLECTABLE_AS_LAST should not
                                         * need the propagation because the
                                         * A collecting the X will occupy
                                         * the location permanently.
                                         */
                                        assert(bit != COLLECTABLE_AS_LAST);
                                        update_movable(map, false, movable);
                                }
                                if (bit == COLLECTABLE_AS_LAST) {
                                        goto update_last_X;
                                }
                                /* restart with the updated movable map */
                                if (verbose) {
                                        printf("retrying after setting "
                                               "[%u:%u] = %02x\n",
                                               loc_x(loc), loc_y(loc), bit);
                                }
                                goto retry;
                        }
                        /*
                         * this X doesn't seem collectable.
                         * we still need to continue processing though
                         * because it's possible this result gets changed
                         * by collecting other X. ("goto retry" above for
                         * the other X.)
                         */
                        all_collectable = false;
                }
        }
        if (last_X > counts[A]) {
                return true;
        }
        return !all_collectable;
}
