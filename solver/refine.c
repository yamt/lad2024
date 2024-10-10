#include <assert.h>

#include "analyze.h"
#include "maputil.h"
#include "refine.h"
#include "solver.h"

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

        calc_movable(map, movable);
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
                        if (objidx == W || is_light(objidx)) {
                                continue;
                        }
                        if (used[loc]) {
                                continue;
                        }
                        enum diridx dir;
                        for (dir = 0; dir < 4; dir++) {
                                loc_t nloc;
                                nloc = loc + dirs[dir].loc_diff;
                                if (map[nloc] != W) {
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
        } while (more);
        return modified;
}
