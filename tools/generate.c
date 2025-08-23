#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "analyze.h"
#include "bb.h"
#include "defs.h"
#include "draw.h"
#include "dump.h"
#include "evaluater.h"
#include "info.h"
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

bool
room(struct genctx *ctx, bool avoid_overlap, bool connect)
{
        const int max = 5;
        const int min = 3;
        const struct bb *bb = &ctx->bb;
        int rw = rng_rand(ctx->rng, min, max);
        // int rh = max + min - rw;
        int rh = rng_rand(ctx->rng, min, max);
        // int rh = rng_rand(ctx->rng, min, max + min - rw);
        if (bb->w < rw + 2 || bb->h < rh + 2) {
                return true;
        }
        int tries = 32;
        do {
                int rx = rng_rand(ctx->rng, bb->x + 1, bb->x + bb->w - rw - 1);
                int ry = rng_rand(ctx->rng, bb->y + 1, bb->y + bb->h - rh - 1);
                if ((!connect ||
                     anyeq(ctx->map, rx - 1, ry - 1, rw + 2, rh + 2, _)) &&
                    (!avoid_overlap || !anyeq(ctx->map, rx, ry, rw, rh, _))) {
                        rect(ctx->map, rx, ry, rw, rh, _);
                        return false;
                }
        } while (tries--);
        return true;
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
        rect(ctx->map, 1, 1, 9, 5, _);
#endif
#if 1
        rect(ctx->map, 1, 1, 8, 8, _);
#endif

#if 0
        rect(ctx->map, 1, 1, 3, 3, _);
        rect(ctx->map, 7, 1, 3, 3, _);
        rect(ctx->map, 7, 7, 3, 3, _);
        rect(ctx->map, 1, 7, 3, 3, _);
        rect(ctx->map, 3, 3, 5, 5, _);
        rect(ctx->map, 5, 5, 1, 1, W);
#endif

#if 0
        {
                int sx = 3;
                int sy = 3;
                rect(ctx->map, 1, (map_height - sy) / 2, map_width - 2, sy, _);
                rect(ctx->map, (map_width - sx) / 2, 1, sx, map_height - 2, _);
                ctx->map[genloc(map_width / 2, map_height / 2)] = W;
        }
#endif

#if 0
        box(ctx->map, 1, 1, map_width - 2, map_height - 2, _);
        box(ctx->map, 2, 2, map_width - 4, map_height - 4, _);
        box(ctx->map, 3, 3, map_width - 6, map_height - 6, _);
#endif

        struct rng *rng __attribute__((__unused__)) = ctx->rng;
        uint8_t *map __attribute__((__unused__)) = ctx->map;
        int i;
        int n;

#if 0
        {
                int x;
                int y;
                int sx = 4;
                int sy = 4;
                for (y = 0; (y + 1) * (sy + 1) + 1 <= map_height; y++) {
                        for (x = 0; (x + 1) * (sx + 1) + 1 <= map_width; x++) {
                                rect(map, 1 + x * (sx + 1), 1 + y * (sy + 1),
                                     4, 4, _);
                                if (x > 0) {
                                        rect(map, 1 + x * (sx + 1) - 1,
                                             1 + y * (sy + 1) + 1, 1, sy - 2,
                                             _);
                                }
                                if (y > 0) {
                                        rect(map, 1 + x * (sx + 1) + 1,
                                             1 + y * (sy + 1) - 1, sx - 2, 1,
                                             _);
                                }
                        }
                }
        }
#endif

#if 0
        /* X */
#define min(a, b) ((a > b) ? (b) : (a))
        {
                int sx = 3;
                int sy = 2;
                int smin = min(map_width - sx, map_height - sy);
                for (i = 1; i < smin - 1; i++) {
                        rect(ctx->map, i, i, sx, sy, _);
                }
        }
        {
                int sx = 2;
                int sy = 3;
                int smin = min(map_width - sx, map_height - sy);
                for (i = 1; i < smin - 1; i++) {
                        rect(ctx->map, smin - i, i, sx, sy, _);
                }
        }
#endif

#if 0
#define min(a, b) ((a > b) ? (b) : (a))
        int r = (min(map_width, map_height) - 3) / 2;
        circle(ctx->map, map_width / 2, map_height / 2, r, _);
        circle(ctx->map, map_width / 2, map_height / 2, r * 2 / 5, W);
#endif

#if 0
        n = rng_rand(rng, 1, 16);
        for (i = 0; i < n; i++) {
                if (room(ctx, false, i > 0)) {
                        return true;
                }
        }
#endif

#if 1
        unsigned int count[END];
        bool connect = false;
        do {
                if (room(ctx, true, connect)) {
                        return true;
                }
                count_objects(ctx->map, count);
                connect = true;
        } while (100 * count[_] / map_size < 20);
#endif

#if 1
        hmirror(ctx->map, ctx->bb.w, ctx->bb.h);
        vmirror(ctx->map, ctx->bb.w, ctx->bb.h);
#endif

#if 0
        random_ichimatsu(ctx);
#endif

#define max_A 4
#define max_P 2

#if 1
        n = rng_rand(rng, -10, max_A);
        if (n <= 0) {
                n = 1;
        }
#else
        n = 1;
#endif
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, A)) {
                        if (i > 0) {
                                break;
                        }
                        return true;
                }
        }
        int max_p = n + max_P;
        if (max_p > max_players) {
                max_p = max_players;
        }
        if (n < max_p) {
#if 1
                n = rng_rand(rng, -10, max_p - n);
                if (n < 0) {
                        n = 1;
                }
#else
                n = 1;
#endif
                for (i = 0; i < n; i++) {
                        if (place_obj(ctx, P)) {
                                if (i > 0) {
                                        break;
                                }
                                return true;
                        }
                }
        }

        struct freq freq[] = {
                {_, 32}, {X, 3}, {B, 3}, {U, 1},
                {R, 1},  {D, 2}, {L, 2}, {W, 16},
        };
        unsigned int nfreq = sizeof(freq) / sizeof(freq[0]);
        if (random_place_objs_in_bb_with_freq(ctx->rng, ctx->map, &ctx->bb,
                                              freq, nfreq)) {
                return true;
        }

        return false;
}

uint64_t
random_seed(void)
{
        uint64_t v;
        arc4random_buf(&v, sizeof(v));
        return v;
}

static void
create_tombstone(uint64_t seed, struct solution *solution)
{
        char filename[100];
        const char *reason = "itr";
        if (solution->giveup_reason == GIVEUP_MEMORY) {
                reason = "mem";
        }

        snprintf(filename, sizeof(filename),
                 "tombstone-%s-moves-%03u-nodes-%u-iterations-%u-seed-%"
                 "016" PRIx64,
                 reason, solution->nmoves, solution->stat.nodes,
                 solution->stat.iterations, seed);
        FILE *fp = fopen(filename, "w");
        if (fp != NULL) {
                fclose(fp);
        }
}

int
main(int argc, char **argv)
{
        bool seed_specified = false;
        uint64_t seed;
        if (argc == 2) {
                char *ep;
                seed = strtoumax(argv[1], &ep, 16);
                seed_specified = true;
        }
        if (!seed_specified) {
                seed = random_seed();
        }
        siginfo_set_message("pid %u main\n", (int)getpid());
        siginfo_setup_handler();
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
#if defined(__APPLE__)
        /*
         * https://developer.apple.com/documentation/apple-silicon/tuning-your-code-s-performance-for-apple-silicon
         */
        pthread_set_qos_class_self_np(QOS_CLASS_UTILITY, 0);
#endif
        const unsigned int score_thresh = 200;
        const unsigned int nmoves_thresh = 50;
        const unsigned int score_thresh2 = 500;
        const unsigned int nmoves_thresh2 = 100;
        uint64_t ntotal = 0;
        uint64_t ngeneratefail = 0;
        uint64_t nsimpleimpossible = 0;
        uint64_t ntsumi = 0;
        uint64_t nimpossible = 0;
        uint64_t ngiveup_memory = 0;
        uint64_t ngiveup_iterations = 0;
        uint64_t nsucceed = 0;
        uint64_t ngood = 0;
        uint64_t ngood2 = 0;
        uint64_t nrefined = 0;
        unsigned int max_iterations_solved = 0;
        unsigned int max_iterations_impossible = 0;
        node_allocator_init();
        time_t start = time(NULL);
        do {
                siginfo_set_message("pid %u generate\n", (int)getpid());
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
                if (tsumi(map)) {
                        if (seed_specified) {
                                dump_map(map);
                                printf("tsumi (seed %" PRIx64 ")\n", seed);
                                exit(1);
                        }
                        ntsumi++;
                        seed++;
                        continue;
                }
                dump_map(map);
                siginfo_set_message("pid %u solving\n", (int)getpid());
                printf("solving (seed %" PRIx64 ")\n", seed);
                struct solution solution;
                unsigned int result =
                        solve("solving", map, &solver_default_param, false,
                              &solution);
                if (result == SOLVE_SOLVED &&
                    validate(map, &solution, false, false)) {
                        /* must be a bug */
                        printf("validation failure\n");
                        validate(map, &solution, true, false);
                        exit(1);
                }
                if (result == SOLVE_SOLVED || result == SOLVE_SOLVABLE) {
                        if (max_iterations_solved < solution.stat.iterations) {
                                max_iterations_solved =
                                        solution.stat.iterations;
                        }
                        unsigned int score = 99999; /* unknown */
                        if (result == SOLVE_SOLVED) {
                                struct evaluation ev;
                                evaluate(map, &solution.moves, &ev);
                                printf("seed %" PRIx64 " score %u\n", seed,
                                       ev.score);
                                score = ev.score;
                        }
                        printf("SEED %" PRIx64 " solved score %u moves %u\n",
                               seed, score, solution.nmoves);
                        if (score >= score_thresh ||
                            solution.nmoves >= nmoves_thresh ||
                            seed_specified) {
                                siginfo_set_message("pid %u refining\n",
                                                    (int)getpid());
                                const char *suffix = "";
                                map_t orig;
                                map_copy(orig, map);
                                if (result == SOLVE_SOLVED &&
                                    try_refine(map, &solution,
                                               &solver_default_param)) {
                                        /*
                                         * XXX: refinement can change the score
                                         */
                                        char filename[100];
                                        snprintf(filename, sizeof(filename),
                                                 "generated-score-%05u-moves-%"
                                                 "03u-seed-%016" PRIx64 ".c",
                                                 score, solution.nmoves, seed);
                                        dump_map_c(map, filename);
                                        suffix = ".orig";
                                        nrefined++;
                                }
                                char filename[100];
                                snprintf(filename, sizeof(filename),
                                         "generated-score-%05u-moves-%03u-"
                                         "seed-%016" PRIx64 ".c%s",
                                         score, solution.nmoves, seed, suffix);
                                dump_map_c(orig, filename);
                                ngood++;
                                if (score >= score_thresh2 ||
                                    solution.nmoves >= nmoves_thresh2) {
                                        ngood2++;
                                }
                        }
                        nsucceed++;
                } else if (result == SOLVE_IMPOSSIBLE) {
                        printf("SEED %" PRIx64 " impossible iterations %u\n",
                               seed, solution.stat.iterations);
                        if (max_iterations_impossible <
                            solution.stat.iterations) {
                                max_iterations_impossible =
                                        solution.stat.iterations;
                        }
                        nimpossible++;
                } else {
                        /* SOLVE_GIVENUP */
                        if (solution.giveup_reason == GIVEUP_MEMORY) {
                                printf("SEED %" PRIx64
                                       " giveup memory moves >=%u\n",
                                       seed, solution.nmoves);
                                ngiveup_memory++;
                                create_tombstone(seed, &solution);
                        } else {
                                printf("SEED %" PRIx64
                                       " giveup iterations %u moves >=%u\n",
                                       seed, solution.stat.iterations,
                                       solution.nmoves);
                                ngiveup_iterations++;
                                create_tombstone(seed, &solution);
                        }
                }
                time_t now = time(NULL);
                time_t dur = now - start;
                printf("total %" PRIu64 " genfail %" PRIu64 " (%.3f)\n"
                       "simple-impossible %" PRIu64 " (%.3f)"
                       " tsumi %" PRIu64 " (%.3f)"
                       " impossible %" PRIu64 " (%.3f)\n"
                       "giveup (mem) %" PRIu64 " (%.3f)"
                       " giveup (iter) %" PRIu64 " (%.3f)\n"
                       "success %" PRIu64 " (%.3f)"
                       " good %" PRIu64 " (%.3f, once %.1f sec)"
                       " good2 %" PRIu64 " (%.3f, once %.1f sec)"
                       " refined %" PRIu64 " (%.3f)\n"
                       "max iter %u (succ) / %u (imp)\n",
                       ntotal, ngeneratefail, (float)ngeneratefail / ntotal,
                       nsimpleimpossible, (float)nsimpleimpossible / ntotal,
                       ntsumi, (float)ntsumi / ntotal, nimpossible,
                       (float)nimpossible / ntotal, ngiveup_memory,
                       (float)ngiveup_memory / ntotal, ngiveup_iterations,
                       (float)ngiveup_iterations / ntotal, nsucceed,
                       (float)nsucceed / ntotal, ngood, (float)ngood / ntotal,
                       (float)dur / ngood, ngood2, (float)ngood2 / ntotal,
                       (float)dur / ngood2, nrefined, (float)nrefined / ntotal,
                       max_iterations_solved, max_iterations_impossible);
                printf("cleaning\n");
                siginfo_set_message("pid %u cleaning\n", (int)getpid());
                clear_solution(&solution);
                solve_cleanup();
                seed++;
        } while (!seed_specified);
}
