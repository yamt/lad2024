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
#include "solver.h"

struct genctx {
        uint8_t *map;
        int h;
        int w;
};

int
gen_rand(int min, int max)
{
        return min + (rand() % (max - min + 1));
}

void
room(struct genctx *ctx)
{
        int x;
        int y;
        int rw = gen_rand(1, 4);
        int rh = gen_rand(1, 4);
        if (ctx->w < rw + 2 || ctx->h < rh + 2) {
                return;
        }
        int rx = gen_rand(1, ctx->w - rw - 1);
        int ry = gen_rand(1, ctx->h - rh - 1);
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
                int x = gen_rand(1, ctx->w - 2);
                int y = gen_rand(1, ctx->h - 2);
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

        room(ctx);
        room(ctx);
        room(ctx);
        room(ctx);
        room(ctx);
        room(ctx);
        return place_obj(ctx, X) || place_obj(ctx, X) || place_obj(ctx, X) ||
               place_obj(ctx, B) || place_obj(ctx, L) || place_obj(ctx, U) ||
               place_obj(ctx, R) || place_obj(ctx, D) || place_obj(ctx, A) ||
               place_obj(ctx, P);
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
                srand(seed);
                struct node *n = alloc_node();
                printf("generating\n");
                ctx.map = n->map;
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
                                dump_map_c(n->map, filename);
                        }
                }
                printf("cleaning\n");
                solve_cleanup();
                printf("cleaned\n");
                seed++;
        }
}
