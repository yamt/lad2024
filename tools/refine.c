#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "analyze.h"
#include "dump.h"
#include "maputil.h"
#include "refine.h"
#include "simplify.h"
#include "solver.h"
#include "validate.h"

/*
 * remove unnecessary spaces.
 * it can also allow further simplification.
 *
 * a possible downside; it might make the game easier by
 * effectively reducing the solution space.
 */
bool
refine(map_t map, const struct solution *solution)
{
        map_t movable;
        map_t reachable;

        calc_movable(map, true, movable);
        bool failed = calc_reachable_from_A(map, movable, reachable);
        assert(!failed);

        map_t used;
        map_fill(used, 0);

        struct node *n;
        LIST_FOREACH(n, &solution->moves, q) {
                used[n->loc] = 1;
                used[next_loc(n)] = 1;
                if ((n->flags & MOVE_PUSH) != 0) {
                        used[pushed_obj_loc(n)] = 1;
                }
        }

        bool modified = false;
        bool more;
        loc_t loc;
        do {
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

                for (loc = 0; loc < map_size; loc++) {
                        if (reachable[loc] == UNREACHABLE) {
                                continue;
                        }
                        uint8_t objidx = map[loc];
                        uint8_t try = objidx;
#if 0 /* disabled because a light can work as an obstacles for P. */
                        if (is_light(objidx)) {
                                if (used[loc]) {
                                        try = B;
                                } else {
                                        try = W;
                                }
                        } else
#endif
                        if (objidx == _ && !used[loc]) {
                                try = W;
                        } else {
                                continue;
                        }
                        map[loc] = try;
                        if (validate(map, solution, false, false)) {
                                map[loc] = objidx;
                        } else {
                                modified = true;
                                more = true;
                        }
                }
        } while (more);
        return modified;
}

bool
try_refine1(map_t map, struct solution *solution,
            const struct solver_param *param)
{
        map_t orig;
        map_copy(orig, map);
        if (!refine(map, solution)) {
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
        detach_solution(solution);
        solve_cleanup();
        struct solution solution_after_refinement;
        unsigned int result =
                solve(map, param, false, &solution_after_refinement);
        if (result != SOLVE_SOLVED ||
            (!removed &&
             solution_after_refinement.nmoves != solution->nmoves) ||
            (removed && solution_after_refinement.nmoves > solution->nmoves)) {
                /* must be a bug */
                printf("refinement changed the solution!\n");
                dump_map(orig);
                dump_map(refinedmap);
                dump_map(map);
                dump_map_c(orig, "refine-bug-orig");
                dump_map_c(map, "refine-bug-refined");
                exit(1);
        }
        clear_solution(&solution_after_refinement);
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
