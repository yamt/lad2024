#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
#include "hash.h"
#include "list.h"
#include "maputil.h"
#include "node.h"
#include "rule.h"
#include "solver.h"

// #define HASH_SIZE 1024021
#define HASH_SIZE 10240033

/* global solver state used by solve()/solve_cleanup() */
struct node_list hash_heads[HASH_SIZE];
struct node_list todo;

bool
add(struct node *n)
{
        uint32_t hash = sdbm_hash(n->map, sizeof(n->map));
        uint32_t idx = hash % HASH_SIZE;
        struct node_list *head = &hash_heads[idx];
        struct node *n2;
        LIST_FOREACH(n2, head, hashq) {
                if (!memcmp(n2->map, n->map, sizeof(n->map))) {
                        return true;
                }
        }
        LIST_INSERT_HEAD(head, n, hashq);
        return false;
}

void
dump_hash(void)
{
#define buckets 10
        unsigned int count[buckets];
        unsigned int total = 0;
        memset(count, 0, sizeof(count));
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                unsigned int n = 0;
                struct node *dn;
                LIST_FOREACH(dn, &hash_heads[i], hashq) {
                        n++;
                }
                if (n >= buckets - 1) {
                        count[buckets - 1]++;
                } else {
                        count[n]++;
                }
                total++;
        }
        printf("hash stat\n");
        for (i = 0; i < buckets; i++) {
                printf("%4d %u (%.3f)\n", i, count[i],
                       (float)count[i] / total);
        }
}

unsigned int
forget_old(unsigned int thresh)
{
        unsigned int removed = 0;
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_list *h = &hash_heads[i];
                struct node *n;
                while ((n = LIST_LAST(h, struct node, hashq)) != NULL) {
                        if (n->steps >= thresh) {
                                break;
                        }
                        LIST_REMOVE(h, n, hashq);
                        free_node(n);
                        removed++;
                }
        }
        return removed;
}

void
return_solution(struct node *n, struct node_list *solution)
{
        LIST_HEAD_INIT(solution);
        do {
                LIST_INSERT_HEAD(solution, n, q);
                n = n->parent;
        } while (n->parent != NULL);
}

unsigned int
solve(struct node *root, const struct solver_param *param, bool verbose,
      struct solution *solution)
{
        LIST_HEAD_INIT(&todo);
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                LIST_HEAD_INIT(&hash_heads[i]);
        }

        size_t limit = param->limit / sizeof(struct node);
        unsigned int queued = 0;
        unsigned int registered = 0;
        unsigned int processed = 0;
        unsigned int duplicated = 0;
        unsigned int ntsumi = 0;

        unsigned int last_thresh = 0;

        root->parent = NULL;
        root->steps = 0;
        LIST_INSERT_TAIL(&todo, root, q);
        queued++;
        add(root);
        registered++;

        unsigned int curstep = 0;
        unsigned int prev_nnodes = 1;
        unsigned int prev_queued = 1;
        struct node *n;
        while ((n = LIST_FIRST(&todo)) != NULL) {
                if (n->steps != curstep) {
                        unsigned int nnodes = queued - prev_queued;
                        printf("step %u nnodes %u (%.3f)\n", n->steps, nnodes,
                               (float)nnodes / prev_nnodes);
                        curstep = n->steps;
                        prev_nnodes = nnodes;
                        prev_queued = queued;
                }
                LIST_REMOVE(&todo, n, q);
                if (tsumi(n->map)) {
                        ntsumi++;
                        goto skip;
                }
                struct stage_meta meta;
                map_t beam_map;
                calc_stage_meta(n->map, &meta);
                assert(meta.nplayers > 0);
                assert(meta.nplayers <= max_players);
                if (n->steps > 0) {
                        /* prefer to continue moving the same player */
                        struct player *p = player_at(
                                &meta, n->loc + dirs[n->dir].loc_diff);
                        assert(p != NULL);
                        if (p != &meta.players[0]) {
                                struct player tmp = meta.players[0];
                                meta.players[0] = *p;
                                *p = tmp;
                        }
                }
                calc_beam(n->map, beam_map);
                unsigned int i;
                for (i = 0; i < meta.nplayers; i++) {
                        struct player *p = &meta.players[i];
                        enum diridx dir;
                        for (dir = 0; dir < 4; dir++) {
                                unsigned int flags =
                                        player_move(&meta, p, dir, n->map,
                                                    beam_map, false);
                                if ((flags & MOVE_OK) == 0) {
                                        continue;
                                }
                                struct node *n2 = alloc_node();
                                map_copy(n2->map, n->map);
                                struct stage_meta meta2 = meta;
                                player_move(&meta2, &meta2.players[i], dir,
                                            n2->map, beam_map, true);
                                if (add(n2)) {
                                        duplicated++;
                                        free_node(n2);
                                        continue;
                                }
                                registered++;
                                n2->parent = n;
                                n2->steps = n->steps + 1;
                                n2->loc = p->loc;
                                n2->dir = dir;
                                n2->flags = flags;
                                if (meta2.nbombs == 0) {
                                        solution->nmoves = n2->steps;
                                        solution->iterations = processed;
                                        if (last_thresh) {
                                                dump_map(n2->map);
                                                printf("proven solvable\n");
                                                return SOLVE_SOLVABLE;
                                        }
                                        printf("solved!\n");
                                        return_solution(n2, &solution->moves);
                                        // dump_hash();
                                        return SOLVE_SOLVED;
                                }
                                LIST_INSERT_TAIL(&todo, n2, q);
                                queued++;
                        }
                }
skip:
                processed++;
                if (verbose && (processed % 100000) == 0) {
                        dump_map(n->map);
                        printf("%u / %u (%u) / %u dup %u (%.3f) tsumi %u step "
                               "%u (%.3f "
                               "left)\n",
                               processed, queued, queued - processed,
                               registered, duplicated,
                               (float)duplicated / processed, ntsumi, n->steps,
                               (float)(prev_queued - processed) / prev_nnodes);
                        // dump_hash();
                }
                if (registered > limit) {
                        /*
                         * note: we keep nodes for previous steps
                         * for two purpose:
                         *  - to build the solution to return
                         *  - for dedup to reduce the space to traverse
                         */
                        unsigned int thresh = n->steps / 2;
                        if (thresh <= last_thresh) {
                                thresh = last_thresh + 1;
                                if (thresh > n->steps) {
                                        printf("too many registered nodes\n");
                                        solution->giveup_reason =
                                                GIVEUP_MEMORY;
                                        return SOLVE_GIVENUP;
                                }
                        }
                        printf("removing old nodes (step thresh %u / %u (%.3f "
                               "left), "
                               "processed %u)\n",
                               thresh, n->steps,
                               (float)(prev_queued - processed) / prev_nnodes,
                               processed);
                        unsigned int removed = forget_old(thresh);
                        printf("removed %u / %u nodes\n", removed, registered);
                        registered -= removed;
                        last_thresh = thresh;
                }
                if (processed >= param->max_iterations) {
                        printf("giving up\n");
                        solution->giveup_reason = GIVEUP_ITERATIONS;
                        return SOLVE_GIVENUP;
                }
        }
        printf("impossible\n");
        solution->iterations = processed;
        return SOLVE_IMPOSSIBLE;
}

void
solve_cleanup(void)
{
#if 1
        free_all_nodes();
#else
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_list *h = &hash_heads[i];
                struct node *n;
                struct node *next;
                for (n = LIST_FIRST(h); n != NULL; n = next) {
                        next = LIST_NEXT(n, hashq);
                        free_node(n);
                }
        }
#endif
}

struct solver_param solver_default_param = {
        .limit = (size_t)8 * 1024 * 1024 * 1024,
        .max_iterations = 100000000,
};
