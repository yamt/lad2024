#include <stdio.h>

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
validate(const map_t omap, const struct solution *solution, bool verbose)
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
        LIST_FOREACH(n, &solution->moves, q) {
                moves++;
                if (verbose) {
                        printf("====== move %u (%d %d %c)\n", moves,
                               loc_x(n->loc), loc_y(n->loc), dirchr(n->dir));
                }
                struct player *p = player_at(&meta, n->loc);
                if (p == NULL) {
                        printf("invalid player %d %d %c\n", loc_x(n->loc),
                               loc_y(n->loc), dirchr(n->dir));
                        return true;
                }
                unsigned int flags =
                        player_move(&meta, p, n->dir, map, beam, true);
                if (n->flags != flags) {
                        printf("invalid move %d %d %c (%x != %x)\n",
                               loc_x(n->loc), loc_y(n->loc), dirchr(n->dir),
                               flags, n->flags);
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
