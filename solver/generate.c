#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bb.h"
#include "defs.h"
#include "draw.h"
#include "dump.h"
#include "evaluater.h"
#include "list.h"
#include "loader.h"
#include "maputil.h"
#include "node.h"
#include "refine.h"
#include "rng.h"
#include "simplify.h"
#include "solver.h"
#include "validate.h"

struct genctx {
        struct rng *rng;
        uint8_t *map;
        struct bb bb;
};

void
room(struct genctx *ctx, bool connect)
{
        const struct bb *bb = &ctx->bb;
        int rw = rng_rand(ctx->rng, 1, 5);
        int rh = 6 - rw;
        if (bb->w < rw + 2 || bb->h < rh + 2) {
                return;
        }
        int tries = 32;
        do {
                int rx = rng_rand(ctx->rng, bb->x + 1, bb->x + bb->w - rw - 1);
                int ry = rng_rand(ctx->rng, bb->y + 1, bb->y + bb->h - rh - 1);
                if (!connect ||
                    anyeq(ctx->map, rx - 1, ry - 1, rw + 2, rh + 2, _)) {
                        rect(ctx->map, rx, ry, rw, rh, _);
                        break;
                }
        } while (tries--);
}

bool
place_obj(struct genctx *ctx, uint8_t objidx)
{
        return random_place_obj_in_bb(ctx->rng, ctx->map, &ctx->bb, simple_put,
                                      &objidx);
}

bool
simple_impossible_check(const map_t map)
{
        unsigned int count[END];
        count_objects(map, count);
#if 0
        unsigned int i;
        for (i = 0; i < END; i++) {
                printf("count[%c] = %u\n", objchr(i), count[i]);
        }
#endif

        if (count[A] == 0) {
                return true;
        }
        if (count[X] == 0) {
                /* this is not impossbile. but not interesting anyway. */
                return true;
        }
        if (count[L] == 0 && count[D] == 0 && count[R] == 0 && count[U] == 0) {
                return true;
        }
        return false;
}

void
random_ichimatsu(struct genctx *ctx)
{
        struct rng *rng = ctx->rng;
        int i;
        loc_t loc;
        for (loc = 0; loc < map_size; loc++) {
                if (ctx->map[loc] == _ && ((loc_x(loc) + loc_y(loc)) & 1)) {
                        ctx->map[loc] = rng_rand(rng, W, U);
                }
        }
}

bool
generate(struct genctx *ctx)
{
        rect(ctx->map, 0, 0, map_width, map_height, W);

#if 0
        rect(ctx->map, 1, 1, 3, 3, _);
        rect(ctx->map, 7, 1, 3, 3, _);
        rect(ctx->map, 7, 7, 3, 3, _);
        rect(ctx->map, 1, 7, 3, 3, _);
        rect(ctx->map, 3, 3, 5, 5, _);
        rect(ctx->map, 5, 5, 1, 1, W);
#endif

        struct rng *rng = ctx->rng;
        int i;
        int n;

#if 0
        n = rng_rand(rng, 1, 16);
        for (i = 0; i < n; i++) {
                room(ctx, i > 0);
        }
#endif
        unsigned int count[END];
        bool connect = false;
        do {
                room(ctx, connect);
                count_objects(ctx->map, count);
                connect = true;
        } while (100 * count[_] / map_size < 30);

#if 0
        random_ichimatsu(ctx);
#endif

        n = rng_rand(rng, -10, max_players);
        if (n <= 0) {
                n = 1;
        }
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, A)) {
                        return true;
                }
        }
        if (n < max_players) {
                n = rng_rand(rng, -10, 4 - n);
                if (n < 0) {
                        n = 1;
                }
                for (i = 0; i < n; i++) {
                        if (place_obj(ctx, P)) {
                                return true;
                        }
                }
        }

        if (random_place_objs_in_bb(ctx->rng, ctx->map, &ctx->bb)) {
                return true;
        }

        return false;
}

bool
try_refine(map_t map, struct solution *solution,
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
                printf("validation failure "
                       "after refinement\n");
                dump_map(orig);
                dump_map(refinedmap);
                dump_map(map);
                exit(1);
        }

#if 1
        solve_cleanup();
        struct solution solution_after_refinement;
        struct node *n = alloc_node();
        map_copy(n->map, map);
        unsigned int result =
                solve(n, param, false, &solution_after_refinement);
        if (result != SOLVE_SOLVED ||
            (!removed &&
             solution_after_refinement.nmoves != solution->nmoves) ||
            (removed && solution_after_refinement.nmoves > solution->nmoves)) {
                printf("refinement changed the solution!\n");
                dump_map(orig);
                dump_map(refinedmap);
                dump_map(map);
                exit(1);
        }
#endif
        align_to_top_left(map);

        return true;
}

uint64_t
random_seed(void)
{
        uint64_t v;
        arc4random_buf(&v, sizeof(v));
        return v;
}

int
main(int argc, char **argv)
{
        struct genctx ctx;
        ctx.bb.x = 0;
        ctx.bb.y = 0;
        ctx.bb.h = map_height;
        ctx.bb.w = map_width;
        if (ctx.bb.w > map_width || ctx.bb.h > map_height) {
                printf("ctx %u %u\n", ctx.bb.w, ctx.bb.h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        uint64_t seed = random_seed();
        uint64_t ntotal = 0;
        uint64_t ngeneratefail = 0;
        uint64_t nsimpleimpossible = 0;
        uint64_t nimpossible = 0;
        uint64_t ngiveup = 0;
        uint64_t nsucceed = 0;
        uint64_t ngood = 0;
        uint64_t nrefined = 0;
        unsigned int max_iterations_solved = 0;
        unsigned int max_iterations_impossible = 0;
        while (1) {
                map_t map;
                struct rng rng;
                rng_init(&rng, seed);
                // printf("generating seed=%" PRIx64 "\n", seed);
                ntotal++;
                ctx.map = map;
                ctx.rng = &rng;
                if (generate(&ctx)) {
                        // printf("generation failed\n");
                        ngeneratefail++;
                        seed++;
                        continue;
                }
                simplify(map);
                align_to_top_left(map);
                if (simple_impossible_check(map)) {
                        nsimpleimpossible++;
                        seed++;
                        continue;
                }
                dump_map(map);
                printf("solving (seed %" PRIx64 ")\n", seed);
                struct node *n = alloc_node();
                map_copy(n->map, map);
                struct solution solution;
                unsigned int result =
                        solve(n, &solver_default_param, false, &solution);
                if (result == SOLVE_SOLVED &&
                    validate(map, &solution, false, false)) {
                        /* must be a bug */
                        printf("validation failure\n");
                        validate(map, &solution, true, false);
                        exit(1);
                }
                if (result == SOLVE_SOLVED || result == SOLVE_SOLVABLE) {
                        if (max_iterations_solved < solution.iterations) {
                                max_iterations_solved = solution.iterations;
                        }
                        unsigned int score = 99999; /* unknown */
                        if (result == SOLVE_SOLVED) {
                                struct evaluation ev;
                                evaluate(n, &solution.moves, &ev);
                                printf("seed %" PRIx64 " score %u\n", seed,
                                       ev.score);
                                score = ev.score;
                        }
                        if (score >= 10) {
                                char filename[100];
                                snprintf(filename, sizeof(filename),
                                         "generated-score-%05u-moves-%03u-"
                                         "seed-%016" PRIx64 ".c",
                                         score, solution.nmoves, seed);
                                dump_map_c(map, filename);
                                ngood++;
                        }
                        nsucceed++;
                        if (result == SOLVE_SOLVED && score >= 10) {
                                if (try_refine(map, &solution,
                                               &solver_default_param)) {
                                        /*
                                         * XXX: refinement can change the score
                                         */
                                        char filename[100];
                                        snprintf(filename, sizeof(filename),
                                                 "generated-score-%05u-moves-%"
                                                 "03u-seed-%016" PRIx64
                                                 "-refined.c",
                                                 score, solution.nmoves, seed);
                                        dump_map_c(map, filename);
                                        nrefined++;
                                }
                        }
                } else if (result == SOLVE_IMPOSSIBLE) {
                        if (max_iterations_impossible < solution.iterations) {
                                max_iterations_impossible =
                                        solution.iterations;
                        }
                        nimpossible++;
                } else {
                        ngiveup++;
                }
                printf("total %" PRIu64 " genfail %" PRIu64
                       " (%.3f) simple-impossible %" PRIu64
                       " (%.3f) impossible %" PRIu64 " (%.3f) giveup %" PRIu64
                       " (%.3f) success %" PRIu64 " (%.3f) good %" PRIu64
                       " (%.3f) refined %" PRIu64
                       " (%.3f) max iter %u (succ) / %u (imp)\n",
                       ntotal, ngeneratefail, (float)ngeneratefail / ntotal,
                       nsimpleimpossible, (float)nsimpleimpossible / ntotal,
                       nimpossible, (float)nimpossible / ntotal, ngiveup,
                       (float)ngiveup / ntotal, nsucceed,
                       (float)nsucceed / ntotal, ngood, (float)ngood / ntotal,
                       nrefined, (float)nrefined / ntotal,
                       max_iterations_solved, max_iterations_impossible);
                printf("cleaning\n");
                solve_cleanup();
                seed++;
        }
}
