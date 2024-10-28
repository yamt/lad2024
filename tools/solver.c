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
add(const map_t root, struct node *n, const map_t node_map)
{
        uint32_t hash = sdbm_hash(node_map, map_size);
        uint32_t idx = hash % HASH_SIZE;
#if defined(NODE_KEEP_HASH)
        n->hash = hash;
#endif
        struct node_list *head = &hash_heads[idx];
        struct node *n2;
        LIST_FOREACH(n2, head, hashq) {
#if defined(NODE_KEEP_HASH)
                if (hash != n2->hash) {
                        continue;
                }
#endif
#if defined(SMALL_NODE)
                map_t n2_map;
                node_expand_map(n2, root, n2_map);
#else
                const uint8_t *n2_map = n2->map;
#endif
                if (!memcmp(n2_map, node_map, map_size)) {
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
return_solution(struct node *n, struct node_list *solution,
                unsigned int thresh)
{
        assert(LIST_EMPTY(solution));
        do {
                if (n->steps <= thresh) {
                        break;
                }
                LIST_INSERT_HEAD(solution, n, q);
                n = n->parent;
        } while (n->parent != NULL);
}

#define BRANCH_HIST_NBUCKETS 10

unsigned int
solve1(const map_t root_map, const struct solver_param *param, bool verbose,
       const map_t goal, struct solution *solution)
{
        solution->detached = false;
        LIST_HEAD_INIT(&solution->moves);

        LIST_HEAD_INIT(&todo);
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                LIST_HEAD_INIT(&hash_heads[i]);
        }

        size_t limit = param->limit / sizeof(struct node);
        struct stats {
                unsigned int queued;
                unsigned int registered;
                unsigned int processed;
                unsigned int duplicated;
                unsigned int ntsumi;
                unsigned int ntsumicheck;
                unsigned int nnodes; /* queued for this step */
        };
        struct stats stats;
        struct stats ostats;
        memset(&stats, 0, sizeof(stats));

        unsigned int curstep = 0;
        unsigned int last_thresh = 0;

        unsigned int branch_hist[BRANCH_HIST_NBUCKETS];
        memset(branch_hist, 0, sizeof(branch_hist));

        struct node *root = alloc_node();
#if !defined(SMALL_NODE)
        map_copy(root->map, root_map);
#endif
        root->parent = NULL;
        root->steps = 0;
        root->flags = 0;
        LIST_INSERT_TAIL(&todo, root, q);
        stats.queued++;
        add(root_map, root, root_map);
        stats.registered++;

        stats.nnodes = 1;

        ostats = stats;
        struct node *n;
        while ((n = LIST_FIRST(&todo)) != NULL) {
                assert(stats.ntsumi <= stats.ntsumicheck);
                assert(stats.processed <= stats.queued);
                if (n->steps != curstep) {
#define D(X) (stats.X - ostats.X)
                        assert(ostats.queued == stats.processed);
                        assert(ostats.nnodes == D(processed));
                        stats.nnodes = D(queued);
                        unsigned int nt = D(ntsumicheck);
                        printf("step %u nnodes %u (%.3f) tsumicheck %.3f "
                               "tsumi %.3f (%.3f)\n",
                               n->steps, stats.nnodes,
                               (float)stats.nnodes / ostats.nnodes,
                               (float)D(ntsumicheck) / ostats.nnodes,
                               (float)D(ntsumi) / ostats.nnodes,
                               (nt > 0) ? (float)D(ntsumi) / nt : (float)0);
                        if (verbose) {
                                printf("branch:");
                                unsigned int i;
                                unsigned int total = 0;
                                for (i = 0; i < BRANCH_HIST_NBUCKETS; i++) {
                                        total += branch_hist[i];
                                }
                                assert(total == D(processed) - D(ntsumi));
                                for (i = 0; i < BRANCH_HIST_NBUCKETS; i++) {
                                        printf(" %u:%.2f", i,
                                               (float)branch_hist[i] / total);
                                }
                                printf("\n");
                        }
                        memset(branch_hist, 0, sizeof(branch_hist));
                        curstep = n->steps;
                        ostats = stats;
                }
                LIST_REMOVE(&todo, n, q);
#if defined(SMALL_NODE)
                map_t map;
                node_expand_map(n, root_map, map);
#else
                uint8_t *map = n->map;
#endif
                map_t beam_map;
                calc_beam(map, beam_map);
                if (!is_trivial(n, map, beam_map)) {
                        stats.ntsumicheck++;
                        if (tsumi(map)) {
                                stats.ntsumi++;
                                goto skip;
                        }
                }
                struct stage_meta meta;
                calc_stage_meta(map, &meta);
                assert(meta.nplayers > 0);
                assert(meta.nplayers <= max_players);
                if (n->steps > 0) {
                        /* prefer to continue moving the same player */
                        struct player *p = player_at(&meta, next_loc(n));
                        assert(p != NULL);
                        if (p != &meta.players[0]) {
                                struct player tmp = meta.players[0];
                                meta.players[0] = *p;
                                *p = tmp;
                        }
                }
                unsigned int nbranches = 0;
                unsigned int i;
                for (i = 0; i < meta.nplayers; i++) {
                        struct player *p = &meta.players[i];
                        enum diridx dir;
                        for (dir = 0; dir < 4; dir++) {
                                unsigned int flags = player_move(
                                        &meta, p, dir, map, beam_map, false);
                                if ((flags & MOVE_OK) == 0) {
                                        continue;
                                }
                                struct node *n2 = alloc_node();
#if defined(SMALL_NODE)
                                map_t map2;
#else
                                uint8_t *map2 = n2->map;
#endif
                                map_copy(map2, map);
                                struct stage_meta meta2 = meta;
                                player_move(&meta2, &meta2.players[i], dir,
                                            map2, beam_map, true);
                                if (add(root_map, n2, map2)) {
                                        stats.duplicated++;
                                        free_node(n2);
                                        continue;
                                }
                                stats.registered++;
                                n2->parent = n;
                                n2->steps = n->steps + 1;
                                n2->loc = p->loc;
                                n2->dir = dir;
                                n2->flags = flags;
                                if (meta2.nbombs == 0) {
                                        if (goal != NULL) {
                                                printf("solved "
                                                       "unexpectedly!\n");
                                                exit(1); /* must be a bug */
                                        }
                                        solution->nmoves = n2->steps;
                                        solution->iterations = stats.processed;
                                        return_solution(n2, &solution->moves,
                                                        last_thresh);
                                        if (last_thresh) {
                                                dump_map(map2);
                                                printf("proven solvable "
                                                       "(steps=%u)\n",
                                                       solution->nmoves);
                                                return SOLVE_SOLVABLE;
                                        }
                                        printf("solved!\n");
                                        // dump_hash();
                                        return SOLVE_SOLVED;
                                } else if (goal != NULL &&
                                           !memcmp(goal, map2, map_size)) {
                                        solution->nmoves = n2->steps;
                                        solution->iterations = stats.processed;
                                        return_solution(n2, &solution->moves,
                                                        last_thresh);
                                        if (last_thresh) {
                                                return SOLVE_SOLVABLE;
                                        }
                                        return SOLVE_SOLVED;
                                }
                                LIST_INSERT_TAIL(&todo, n2, q);
                                stats.queued++;
                                nbranches++;
                        }
                }
                assert(nbranches <= 4 * meta.nplayers);
                if (nbranches >= BRANCH_HIST_NBUCKETS - 1) {
                        branch_hist[BRANCH_HIST_NBUCKETS - 1]++;
                } else {
                        branch_hist[nbranches]++;
                }
skip:
                stats.processed++;
                if (verbose && (stats.processed % 100000) == 0) {
                        dump_map(map);
                        printf("%u / %u (%u) / %u dup %u (%.3f) tsumi %u/%u "
                               "(%.3f) "
                               "step "
                               "%u (%.3f "
                               "left)\n",
                               stats.processed, stats.queued,
                               stats.queued - stats.processed,
                               stats.registered, stats.duplicated,
                               (float)stats.duplicated / stats.processed,
                               stats.ntsumi, stats.ntsumicheck,
                               (float)stats.ntsumi / stats.ntsumicheck,
                               n->steps,
                               (float)(ostats.queued - stats.processed) /
                                       stats.nnodes);
                        // dump_hash();
                }
                if (stats.registered > limit) {
#if defined(SMALL_NODE)
                        printf("too many registered nodes\n");
                        solution->giveup_reason = GIVEUP_MEMORY;
                        return SOLVE_GIVENUP;
#else
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
                               (float)(ostats.queued - stats.processed) /
                                       ostats.nnodes,
                               stats.processed);
                        unsigned int removed = forget_old(thresh);
                        printf("removed %u / %u nodes\n", removed,
                               stats.registered);
                        stats.registered -= removed;
                        last_thresh = thresh;
#endif
                }
                if (stats.processed >= param->max_iterations) {
                        printf("giving up\n");
                        solution->giveup_reason = GIVEUP_ITERATIONS;
                        return SOLVE_GIVENUP;
                }
        }
        printf("impossible\n");
        solution->iterations = stats.processed;
        return SOLVE_IMPOSSIBLE;
}

unsigned int
solve(const map_t map, const struct solver_param *param, bool verbose,
      struct solution *solution)
{
        unsigned int result;
        result = solve1(map, param, verbose, NULL, solution);
#if !defined(SMALL_NODE)
        while (result == SOLVE_SOLVABLE) {
                {
                        struct node *n = LIST_FIRST(&solution->moves);
                        printf("step %u (the first known state):\n", n->steps);
                        dump_map(n->map);
                }
                {
                        struct node *n =
                                LIST_LAST(&solution->moves, struct node, q);
                        printf("step %u (the final state):\n", n->steps);
                        dump_map(n->map);
                }

                detach_solution(solution);
                solve_cleanup();
                struct node *n = LIST_FIRST(&solution->moves);
                assert(n->steps > 1);
                struct solution solution2;
                map_t goal;
                prev_map(n, n->map, goal);
                printf("re-solving up to the intermediate goal (step %u):\n",
                       n->steps - 1);
                dump_map(goal);
                result = solve1(map, param, false, goal, &solution2);
                if (result != SOLVE_SOLVED && result != SOLVE_SOLVABLE) {
                        printf("solve() returned an unexpected result %u\n",
                               result);
                        exit(1); /* must be a bug */
                }
                assert(LIST_FIRST(&solution->moves)->steps ==
                       LIST_LAST(&solution2.moves, struct node, q)->steps + 1);
                detach_solution(&solution2);
                solve_cleanup();
                assert(solution->detached == solution2.detached);
                LIST_SPLICE_HEAD(&solution->moves, &solution2.moves, q);
                {
                        struct node t;
                        LIST_INSERT_TAIL(&solution->moves, &t, q);
                        LIST_REMOVE(&solution->moves, &t, q);
                }
        }
#endif
        assert(result != SOLVE_SOLVED ||
               LIST_FIRST(&solution->moves)->steps == 1);
        return result;
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

void
detach_solution(struct solution *solution)
{
        if (solution->detached) {
                return;
        }
        /*
         * make a copy of nodes using malloc so that it's independent
         * from the solver state (thus usable after solve_cleanup)
         */
        struct node_list h;
        LIST_HEAD_INIT(&h);
        struct node *n;
        LIST_FOREACH(n, &solution->moves, q) {
                struct node *nn = malloc(sizeof(*nn));
                *nn = *n;
                nn->parent = NULL;
                LIST_INSERT_TAIL(&h, nn, q);
        }
        LIST_HEAD_INIT(&solution->moves);
        LIST_SPLICE_TAIL(&solution->moves, &h, q);
        solution->detached = true;
}

void
clear_solution(struct solution *solution)
{
        if (!solution->detached) {
                return;
        }
        /*
         * free malloc'ed nodes
         */
        struct node *n;
        while ((n = LIST_FIRST(&solution->moves)) != NULL) {
                assert(n->parent == NULL);
                LIST_REMOVE(&solution->moves, n, q);
                free(n);
        }
}

#if !defined(MEMLIMIT_GB)
#define MEMLIMIT_GB 8
#endif
#if !defined(MAX_ITERATIONS)
#define MAX_ITERATIONS 100000000
#endif

struct solver_param solver_default_param = {
        .limit = (size_t)MEMLIMIT_GB * 1024 * 1024 * 1024,
        .max_iterations = MAX_ITERATIONS,
};
