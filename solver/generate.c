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
#include "node.h"
#include "rng.h"
#include "solver.h"

struct size {
        int xmin;
        int ymin;
        int xmax;
        int ymax;
};

void
measure_size(const map_t map, struct size *size)
{
        size->xmin = map_width - 1;
        size->ymin = map_height - 1;
        size->xmax = 0;
        size->ymax = 0;
        int x;
        int y;
        for (y = 0; y < map_height; y++) {
                for (x = 0; x < map_width; x++) {
                        if (map[genloc(x, y)] != _) {
                                if (size->xmin > x) {
                                        size->xmin = x;
                                }
                                if (size->ymin > y) {
                                        size->ymin = y;
                                }
                                if (size->xmax < x) {
                                        size->xmax = x;
                                }
                                if (size->ymax < y) {
                                        size->ymax = y;
                                }
                        }
                }
        }
}

void
simplify(map_t map)
{
        struct size size;
        measure_size(map, &size);
        map_t orig;
        memcpy(orig, map, map_width * map_height);
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                if (map[loc] != W) {
                        continue;
                }
                enum diridx dir;
                for (dir = 0; dir < 4; dir++) {
                        loc_t nloc = loc + dirs[dir].loc_diff;
                        if (!in_map(nloc)) {
                                continue;
                        }
                        int nx = loc_x(nloc);
                        int ny = loc_y(nloc);
                        if (nx < size.xmin || nx > size.xmax ||
                            ny < size.ymin || ny > size.ymax) {
                                continue;
                        }
                        if (orig[nloc] == W) {
                                continue;
                        }
                        break;
                }
                if (dir == 4) {
                        map[loc] = _;
                }
        }
        measure_size(map, &size);
        if (size.xmin > 0 || size.ymin > 0) {
                loc_t off = genloc(size.xmin, size.ymin);
                memmove(&map[0], &map[off], map_width * map_height - off);
                memset(&map[map_width * map_height - off], 0, off);
        }
}

struct genctx {
        struct rng *rng;
        uint8_t *map;
        int h;
        int w;
};

void
room(struct genctx *ctx)
{
        int x;
        int y;
        int rw = rng_rand(ctx->rng, 1, 4);
        int rh = rng_rand(ctx->rng, 1, 4);
        if (ctx->w < rw + 2 || ctx->h < rh + 2) {
                return;
        }
        int rx = rng_rand(ctx->rng, 1, ctx->w - rw - 1);
        int ry = rng_rand(ctx->rng, 1, ctx->h - rh - 1);
        for (y = 0; y < rh; y++) {
                for (x = 0; x < rw; x++) {
                        ctx->map[genloc(rx + x, ry + y)] = _;
                }
        }
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
generate(struct genctx *ctx)
{
        memset(ctx->map, 0, genloc(map_width, map_height));
        int x;
        int y;
        for (y = 0; y < ctx->h; y++) {
                for (x = 0; x < ctx->w; x++) {
                        ctx->map[genloc(x, y)] = W;
                }
        }

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
        while (1) {
                struct rng rng;
                rng_init(&rng, seed);
                struct node *n = alloc_node();
                printf("generating\n");
                ctx.map = n->map;
                ctx.rng = &rng;
                if (generate(&ctx)) {
                        printf("generation failed\n");
                        seed++;
                        continue;
                }
                printf("generated\n");
                dump_map(n->map);
                printf("solving\n");
                struct node_list solution;
                unsigned int result = solve(n, 10000000, false, &solution);
                if (result == SOLVE_SOLVED) {
                        struct evaluation ev;
                        evaluate(&solution, &ev);
                        printf("seed %u score %u\n", seed, ev.score);
                        if (ev.score >= 10) {
                                char filename[100];
                                snprintf(filename, sizeof(filename),
                                         "generated-score-%u-seed-%u.c",
                                         ev.score, seed);
                                simplify(n->map);
                                dump_map_c(n->map, filename);
                        }
                }
                printf("cleaning\n");
                solve_cleanup();
                printf("cleaned\n");
                seed++;
        }
}
