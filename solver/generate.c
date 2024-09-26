#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "evaluater.h"
#include "list.h"
#include "loader.h"
#include "mapsize.h"
#include "node.h"
#include "rng.h"
#include "simplify.h"
#include "solver.h"

struct genctx {
        struct rng *rng;
        uint8_t *map;
        int h;
        int w;
};

void
rect(map_t map, int rx, int ry, int rw, int rh, uint8_t objidx)
{
        int x;
        int y;
        for (y = 0; y < rh; y++) {
                for (x = 0; x < rw; x++) {
                        map[genloc(rx + x, ry + y)] = objidx;
                }
        }
}

void
room(struct genctx *ctx)
{
        int rw = rng_rand(ctx->rng, 1, 4);
        int rh = rng_rand(ctx->rng, 1, 4);
        if (ctx->w < rw + 2 || ctx->h < rh + 2) {
                return;
        }
        int rx = rng_rand(ctx->rng, 1, ctx->w - rw - 1);
        int ry = rng_rand(ctx->rng, 1, ctx->h - rh - 1);
        rect(ctx->map, rx, ry, rw, rh, _);
}

bool
place_obj(struct genctx *ctx, uint8_t objidx)
{
        int tries = 5;
        do {
                int x = rng_rand(ctx->rng, 1, ctx->w - 2);
                int y = rng_rand(ctx->rng, 1, ctx->h - 2);
                loc_t loc = genloc(x, y);
                if (ctx->map[loc] == _) {
                        ctx->map[loc] = objidx;
                        return false;
                }
        } while (tries--);
        return true;
}

bool
simple_impossible_check(const map_t map)
{
        unsigned int count[END];
        memset(count, 0, sizeof(count));
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                uint8_t objidx = map[loc];
                count[objidx]++;
        }
        unsigned int i;
        for (i = 0; i < END; i++) {
                printf("count[%c] = %u\n", objchr(i), count[i]);
        }

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

bool
generate(struct genctx *ctx)
{
        rect(ctx->map, 0, 0, map_width, map_height, W);

        struct rng *rng = ctx->rng;
        int i;
        int n;
        n = rng_rand(rng, 1, 6);
        for (i = 0; i < n; i++) {
                room(ctx);
        }
        n = rng_rand(rng, 1, 5);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, X)) {
                        return true;
                }
        }
        n = rng_rand(rng, 0, 2);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, B)) {
                        return true;
                }
        }
        n = rng_rand(rng, 0, 2);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, U)) {
                        return true;
                }
        }
        n = rng_rand(rng, 0, 2);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, R)) {
                        return true;
                }
        }
        n = rng_rand(rng, 0, 2);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, D)) {
                        return true;
                }
        }
        n = rng_rand(rng, 0, 2);
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, L)) {
                        return true;
                }
        }
        n = rng_rand(rng, -4, 2);
        if (n <= 0) {
                n = 1;
        }
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, A)) {
                        return true;
                }
        }
        n = rng_rand(rng, -4, 4 - n);
        if (n <= 0) {
                n = 1;
        }
        for (i = 0; i < n; i++) {
                if (place_obj(ctx, P)) {
                        return true;
                }
        }
        simplify(ctx->map);
        if (simple_impossible_check(ctx->map)) {
                return true;
        }
        return false;
}

int
main(int argc, char **argv)
{
        struct genctx ctx;
        ctx.h = 8;
        ctx.w = 8;
        if (ctx.w > map_width || ctx.h > map_height) {
                printf("ctx %u %u\n", ctx.w, ctx.h);
                printf("macro %u %u\n", map_width, map_height);
                exit(1);
        }
        int seed = 0;
        unsigned int ntotal = 0;
        unsigned int ngeneratefail = 0;
        unsigned int nimpossible = 0;
        unsigned int ngiveup = 0;
        unsigned int nsucceed = 0;
        unsigned int ngood = 0;
        while (1) {
                map_t map;
                struct rng rng;
                rng_init(&rng, seed);
                printf("generating seed=%u\n", seed);
                ntotal++;
                ctx.map = map;
                ctx.rng = &rng;
                if (generate(&ctx)) {
                        printf("generation failed\n");
                        ngeneratefail++;
                        seed++;
                        continue;
                }
                dump_map(map);
                printf("solving\n");
                struct node *n = alloc_node();
                memcpy(n->map, map, map_width * map_height);
                struct solution solution;
                size_t limit = (size_t)4 * 1024 * 1024 * 1024; /* 4GB */
                unsigned int result = solve(n, limit, true, &solution);
                if (result == SOLVE_SOLVED || result == SOLVE_SOLVABLE) {
                        unsigned int score = 99999; /* unknown */
                        if (result == SOLVE_SOLVED) {
                                struct evaluation ev;
                                evaluate(n, &solution.moves, &ev);
                                printf("seed %u score %u\n", seed, ev.score);
                                score = ev.score;
                        }
                        if (score >= 10) {
                                char filename[100];
                                snprintf(filename, sizeof(filename),
                                         "generated-score-%05u-moves-%03u-seed-%08x.c",
                                         score, solution.nmoves, seed);
                                simplify(map);
                                dump_map_c(map, filename);
                                ngood++;
                        }
                        nsucceed++;
                } else if (result == SOLVE_IMPOSSIBLE) {
                        nimpossible++;
                } else {
                        ngiveup++;
                }
                printf("total %u genfail %u (%.3f) impossible %u (%.3f) "
                       "giveup %u (%.3f) success %u (%.3f) good %u (%.3f)\n",
                       ntotal, ngeneratefail, (float)ngeneratefail / ntotal,
                       nimpossible, (float)nimpossible / ntotal, ngiveup,
                       (float)ngiveup / ntotal, nsucceed,
                       (float)nsucceed / ntotal, ngood, (float)ngood / ntotal);
                printf("cleaning\n");
                solve_cleanup();
                seed++;
        }
}
