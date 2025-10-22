#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "analyze.h"
#include "dump.h"
#include "evaluater.h"
#include "maputil.h"
#include "refine.h"
#include "simplify.h"
#include "solver.h"
#include "validate.h"

static void
update_solution(struct solution *solution, struct solution *new_solution)
{
        printf("UPDATE SOLUTION %d -> %d\n", solution->nmoves,
               new_solution->nmoves);
        clear_solution(solution);
        /* note: slist head is copyable */
        uint64_t id = solution->id;
        *solution = *new_solution;
        solution->id = id; /* keep the id */
}

/*
 * remove unnecessary spaces.
 * it can also allow further simplification.
 *
 * a possible downside; it might make the game easier by
 * effectively reducing the solution space.
 */
bool
refine(map_t map, bool eager, struct solution *solution,
       const struct solver_param *param)
{

        bool modified = false;
        bool recalc = true;
        bool more;
        loc_t loc;
        do {
                map_t movable;
                map_t reachable;
                map_t used;

                if (recalc) {
                        calc_movable(map, true, movable);
                        bool failed =
                                calc_reachable_from_A(map, movable, reachable);
                        assert(!failed);

                        map_fill(used, 0);

                        struct node *n;
                        SLIST_FOREACH(n, &solution->moves, q) {
                                used[n->loc] = 1;
                                used[next_loc(n)] = 1;
                                if ((n->flags & MOVE_PUSH) != 0) {
                                        used[pushed_obj_loc(n)] = 1;
                                }
                        }
                        recalc = false;
                }

                more = false;
                for (loc = 0; loc < map_size; loc++) {
                        if (reachable[loc] == UNREACHABLE) {
                                continue;
                        }
                        uint8_t objidx = map[loc];
                        if (objidx == W) {
                                continue;
                        }
                        if (is_light(objidx) &&
                            !permanently_blocked(map, movable, loc)) {
                                continue;
                        }
                        if (used[loc]) {
                                continue;
                        }
                        if (objidx == B) {
                                map[loc] = W;
                                continue;
                        }
                        enum diridx dir;
                        for (dir = 0; dir < 4; dir++) {
                                loc_t nloc;
                                nloc = loc + dirs[dir].loc_diff;
                                if (in_map(nloc) && map[nloc] != W) {
                                        continue;
                                }
                                nloc = loc + dirs[(dir + 1) % 4].loc_diff;
                                if (in_map(nloc) && map[nloc] != W) {
                                        continue;
                                }
                                map[loc] = W;
                                modified = true;
                                more = true;
                                break;
                        }
                }

                if (eager) {
                        for (loc = 0; loc < map_size; loc++) {
                                if (reachable[loc] == UNREACHABLE) {
                                        continue;
                                }
                                uint8_t objidx = map[loc];
                                uint8_t try = objidx;
                                if (is_light(objidx)) {
                                        if (used[loc]) {
                                                try = B;
                                        } else {
                                                try = W;
                                        }
                                } else if (objidx == _ && !used[loc]) {
                                        try = W;
                                } else {
                                        continue;
                                }
                                map[loc] = try;
                                /*
                                 * always perform slow validation here because
                                 * fast validation might fail even when
                                 * the refinement attepmt improved the stage.
                                 */
                                struct solution new_solution;
                                if (/* validate(map, solution, false, false) ||
                                     */
                                    validate_slow(map, solution, param, false,
                                                  false, &new_solution)) {
                                        map[loc] = objidx; /* revert */
                                } else {
                                        modified = true;
                                        more = true;
                                        update_solution(solution,
                                                        &new_solution);
                                        struct evaluation ev;
                                        evaluate(map, &solution->moves, true,
                                                 &ev);
                                        recalc = true;
                                }
                        }
                }
        } while (more);
        return modified;
}

bool
try_refine1(map_t map, struct solution *solution,
            const struct solver_param *param)
{
        detach_solution(solution);
        map_t orig;
        map_copy(orig, map);
        if (!refine(map, true, solution, param)) {
                return false;
        }
        map_t refinedmap;
        map_copy(refinedmap, map);
        unsigned int count1[END];
        unsigned int count2[END];
        count_objects(map, count1);
        simplify(map);
        count_objects(map, count2);
        if (count2[X] == 0) {
                return false;
        }
        bool removed = count1[A] > count2[A] || count1[P] > count2[P];
        if (validate(map, solution, false, removed)) {
                /* must be a bug */
                printf("validation failure after refinement\n");
                dump_map(orig);
                dump_map(refinedmap);
                dump_map(map);
                exit(1);
        }

#if 1
        struct solution alt_solution;
        if (validate_slow(map, solution, param, false, removed,
                          &alt_solution)) {
                /*
                 * must be a bug unless USE_BLOOM_FILTER.
                 *
                 * with USE_BLOOM_FILTER,
                 * as a bloom filter allows false positives,
                 * any changes to the stage can make it unsolvable.
                 * it's ok as far as it's rare.
                 * we still dump it for later examination though.
                 */
                printf("refinement changed the solution!\n");
                printf("====== orig\n");
                dump_map(orig);
                printf("====== refined\n");
                dump_map(refinedmap);
                printf("====== simplified\n");
                dump_map(map);
                dump_map_c_fmt(orig, "refine-bug-%" PRIx64 "-orig",
                               solution->id);
                dump_map_c_fmt(map, "refine-bug-%" PRIx64 "-refined",
                               solution->id);
#if !defined(USE_BLOOM_FILTER)
                exit(1);
#endif
        } else {
                update_solution(solution, &alt_solution);
        }
#endif
        return true;
}

bool
try_refine(map_t map, struct solution *solution,
           const struct solver_param *param)
{
        bool modified = false;
        while (try_refine1(map, solution, param)) {
                modified = true;
        }
        align_to_top_left(map);
        return modified;
}
