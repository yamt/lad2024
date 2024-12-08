#include <assert.h>
#include <stdio.h>

#include "analyze.h"
#include "dump.h"
#include "maputil.h"
#include "node.h"
#include "rule.h"
#include "solver.h"
#include "validate.h"

/*
 * validate a solution.
 * expected to be quicker than solving it again.
 */
bool
validate(const map_t omap, const struct solution *solution, bool verbose,
         bool allow_removed_players)
{
        struct stage_meta meta;
        map_t map;
        map_t beam;

        map_copy(map, omap);
        calc_beam(map, beam);
        calc_stage_meta(map, &meta);

        if (verbose) {
                printf("====== start validating a solution\n");
                dump_map(map);
        }
        unsigned int moves = 0;
        struct node *n;
        SLIST_FOREACH(n, &solution->moves, q) {
                moves++;
                if (verbose) {
                        printf("====== move %u (%d %d %c)\n", moves,
                               loc_x(n->loc), loc_y(n->loc), dirchr(n->dir));
                }
                struct player *p = player_at(&meta, n->loc);
                if (p == NULL) {
                        printf("invalid player %d %d %c\n", loc_x(n->loc),
                               loc_y(n->loc), dirchr(n->dir));
                        /*
                         * note: for the stage originally with multiple As,
                         * sometimes refine+simplify eliminate sets of A and
                         * Xs. in that case, the simplified stage should
                         * still be solvable by ignoring moves of the
                         * eliminated A, which has been turned into W. (or
                         * even _.)
                         */
                        if (allow_removed_players &&
                            (map[n->loc] == W || map[n->loc] == _)) {
                                continue;
                        }
                        return true;
                }
                move_flags_t flags =
                        player_move(&meta, p, n->dir, map, beam, true);
                if (n->flags != flags) {
                        printf("invalid move %d %d %c (%x != %x)\n",
                               loc_x(n->loc), loc_y(n->loc), dirchr(n->dir),
                               flags, n->flags);
                        return true;
                }
                if (tsumi(map)) {
                        printf("unexpected tsumi during validation!\n");
                        dump_map(map);
                        return true;
                }
                if (verbose) {
                        printf("====== move %u ok\n", moves);
                        dump_map(map);
                }
                if ((flags & MOVE_BEAM) != 0) {
                        calc_beam(map, beam);
                }
        }
        if (meta.nbombs != 0) {
                printf("didn't collect all bombs\n");
                return true;
        }
        if (solution->nmoves != moves) {
                printf("wrong nmoves\n");
                return true;
        }
        if (verbose) {
                printf("====== result ok\n");
                dump_map(map);
        }
        return false;
}

/*
 * validate a solution by solving the stage again.
 */
bool
validate_slow(const map_t map, const struct solution *solution,
              const struct solver_param *param, bool verbose,
              bool allow_removed_players)
{
        assert(solution->detached);
        solve_cleanup();
        struct solution solution_after_refinement;
        unsigned int result =
                solve(map, param, false, &solution_after_refinement);
        clear_solution(&solution_after_refinement);
        if (result != SOLVE_SOLVED ||
            (!allow_removed_players &&
             solution_after_refinement.nmoves != solution->nmoves) ||
            (allow_removed_players &&
             solution_after_refinement.nmoves > solution->nmoves)) {
                return true;
        }
        return false;
}
