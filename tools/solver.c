#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
#include "hash.h"
#include "info.h"
#include "list.h"
#include "maputil.h"
#include "node.h"
#include "rule.h"
#include "sha256.h"
#include "solver.h"

// #define HASH_SIZE 1024021
#define HASH_SIZE 10240033

/* global solver state used by solve()/solve_cleanup() */
#if defined(SMALL_NODE)
#define node_hashlist node_slist
#else
#define node_hashlist node_list
#endif
struct node_hashlist hash_heads[HASH_SIZE];
struct node_slist todo;

#if defined(USE_BLOOM_FILTER)
#if !defined(BLOOM_FILTER_SIZE)
#define BLOOM_FILTER_SIZE (1024 * 1024 * 1024)
#endif
static uint32_t filter[(BLOOM_FILTER_SIZE + 32 - 1) / 32];
static struct {
        uint32_t add;
        uint32_t n;
} bloom_filter_stats;
#if !defined(BLOOM_FILTER_K)
#define BLOOM_FILTER_K 4
#endif

bool
add_filter(const uint32_t h[8])
{
        bool all = true;
        unsigned int i;
        for (i = 0; i < BLOOM_FILTER_K; i++) {
                uint32_t bitidx = h[i] % BLOOM_FILTER_SIZE;
                uint32_t idx = bitidx / 32;
                uint32_t mask = UINT32_C(1) << (bitidx % 32);
                if ((filter[idx] & mask) == 0) {
                        all = false;
                        filter[idx] |= mask;
                }
        }
        bloom_filter_stats.add++;
        if (!all) {
                bloom_filter_stats.n++;
        }
        return all;
}
#endif

bool
add(const map_t root, struct node *n, const map_t node_map)
{
#if defined(USE_BLOOM_FILTER)
        uint32_t h[8];
        sha256(node_map, map_size, h);
        if (add_filter(h)) {
                return true;
        }
        uint32_t hash = h[0];
        uint32_t idx = h[1] % HASH_SIZE;
#else
        uint32_t hash = sdbm_hash(node_map, map_size);
        uint32_t idx = hash % HASH_SIZE;
#endif
#if defined(NODE_KEEP_HASH)
        n->hash = hash;
#endif
        struct node_hashlist *head = &hash_heads[idx];
        struct node *n2;
#if defined(SMALL_NODE)
        SLIST_FOREACH(n2, head, hashq) {
#else
        LIST_FOREACH(n2, head, hashq) {
#endif
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
#if defined(USE_BLOOM_FILTER)
                        abort(); /* false negative is a bug */
#else
                        return true;
#endif
                }
        }
#if defined(SMALL_NODE)
        SLIST_INSERT_HEAD(head, n, hashq);
#else
        LIST_INSERT_HEAD(head, n, hashq);
#endif
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
#if defined(SMALL_NODE)
                SLIST_FOREACH(dn, &hash_heads[i], hashq) {
#else
                LIST_FOREACH(dn, &hash_heads[i], hashq) {
#endif
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

#if !defined(SMALL_NODE)
unsigned int
forget_old(unsigned int thresh)
{
        unsigned int removed = 0;
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_hashlist *h = &hash_heads[i];
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
#endif

#if defined(SMALL_NODE)
unsigned int
forget_unreachable(const struct node_slist *todo)
{
        unsigned int removed = 0;
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_hashlist *h = &hash_heads[i];
                struct node *n;
                SLIST_FOREACH(n, h, hashq) {
                        n->flags |= 0x80;
                }
        }
        struct node *n;
        SLIST_FOREACH(n, todo, q) {
                struct node *m;
                for (m = n; m != NULL; m = m->parent) {
                        if ((m->flags & 0x80) == 0) {
                                break;
                        }
                        m->flags &= ~0x80;
                }
        }
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_hashlist *h = &hash_heads[i];
                struct node *n;
                struct node *next;
                struct node *prev = NULL;
                for (n = SLIST_FIRST(h); n != NULL; n = next) {
                        next = SLIST_NEXT(n, hashq);
                        if ((n->flags & 0x80) == 0) {
                                prev = n;
                                continue;
                        }
                        SLIST_REMOVE(h, prev, n, hashq);
                        free_node(n);
                        removed++;
                }
        }
        return removed;
}
#endif

void
return_solution(struct node *n, struct node_slist *solution,
                unsigned int thresh)
{
        assert(SLIST_EMPTY(solution));
        do {
                if (n->steps <= thresh) {
                        break;
                }
                SLIST_INSERT_HEAD(solution, n, q);
                n = n->parent;
        } while (n->parent != NULL);
}

#define BRANCH_HIST_NBUCKETS 10

unsigned int
solve1(const char *msg, const map_t root_map, const struct solver_param *param,
       bool verbose, const map_t goal, struct solution *solution)
{
        time_t start = time(NULL);
        siginfo_clear_message();
        solution->detached = false;
        SLIST_HEAD_INIT(&solution->moves);

        SLIST_HEAD_INIT(&todo);
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
#if defined(SMALL_NODE)
                SLIST_HEAD_INIT(&hash_heads[i]);
#else
                LIST_HEAD_INIT(&hash_heads[i]);
#endif
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
        unsigned int last_gc = 0;

        unsigned int branch_hist[BRANCH_HIST_NBUCKETS];
        memset(branch_hist, 0, sizeof(branch_hist));

        struct node *root = alloc_node();
#if !defined(SMALL_NODE)
        map_copy(root->map, root_map);
#endif
        root->parent = NULL;
        root->steps = 0;
        root->flags = 0;
        SLIST_INSERT_TAIL(&todo, root, q);
        stats.queued++;
        add(root_map, root, root_map);
        stats.registered++;
        solution->stat.nodes++;

        stats.nnodes = 1;

        ostats = stats;
        struct node *n;
        struct node *prevnode = NULL;
#if defined(SMALL_NODE)
        map_t map;
#endif
        while ((n = SLIST_FIRST(&todo)) != NULL) {
                if (siginfo_latch_pending()) {
                        time_t now = time(NULL);
                        fprintf(stderr,
                                "pid %u %s step %u itr %u (%.1lf /s) (%.1lf "
                                "%%) mem %u "
                                "(%.2lf MB)\n",
                                (int)getpid(), msg, n->steps, stats.processed,
                                (double)stats.processed / (now - start),
                                (double)stats.processed / stats.registered *
                                        100,
                                stats.registered,
                                (double)sizeof(struct node) *
                                        stats.registered / 1024 / 1024);
#if defined(USE_BLOOM_FILTER)
                        double optimal_k =
                                0.7 * BLOOM_FILTER_SIZE / bloom_filter_stats.n;
                        double false_pos_prob = pow(0.5, optimal_k);
                        fprintf(stderr,
                                "pid %u bloom filter n=%" PRIu32
                                " optimal-K=%.1f false-pos-prob=%f\n",
                                (int)getpid(), bloom_filter_stats.n, optimal_k,
                                false_pos_prob);
#endif
                }
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
                        // dump_hash();
                }
                SLIST_REMOVE(&todo, (struct node *)NULL, n, q);
#if defined(SMALL_NODE)
                if (prevnode != NULL && prevnode->parent == n->parent) {
                        node_undo(prevnode, map);
                        node_apply(n, map);
                } else {
                        node_expand_map(n, root_map, map);
                }
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
                                move_flags_t flags = player_move(
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
                                solution->stat.nodes++;
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
                                        solution->stat.iterations +=
                                                stats.processed;
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
                                        solution->stat.iterations +=
                                                stats.processed;
                                        return_solution(n2, &solution->moves,
                                                        last_thresh);
                                        if (last_thresh) {
                                                return SOLVE_SOLVABLE;
                                        }
                                        return SOLVE_SOLVED;
                                }
                                SLIST_INSERT_TAIL(&todo, n2, q);
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
#if defined(SMALL_NODE)
                prevnode = n;
#endif
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
                        unsigned int step = n->steps;
                        if (last_gc == step) {
                                printf("too many registered nodes (%u)\n",
                                       stats.registered);
                                solution->giveup_reason = GIVEUP_MEMORY;
                                solution->nmoves = n->steps;
                                solution->stat.iterations += stats.processed;
                                return SOLVE_GIVENUP;
                        }
                        printf("removing unreachable nodes\n");
                        unsigned int removed = forget_unreachable(&todo);
                        printf("removed %u / %u nodes (%.3f)\n", removed,
                               stats.registered,
                               (float)removed / stats.registered);
                        stats.registered -= removed;
                        last_gc = step;
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
                                        solution->nmoves = n->steps;
                                        solution->stat.iterations +=
                                                stats.processed;
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
                        solution->stat.iterations += stats.processed;
                        solution->nmoves = n->steps;
                        return SOLVE_GIVENUP;
                }
        }
        printf("impossible\n");
        solution->stat.iterations += stats.processed;
        return SOLVE_IMPOSSIBLE;
}

unsigned int
solve(const char *msg, const map_t map, const struct solver_param *param,
      bool verbose, struct solution *solution)
{
        memset(&solution->stat, 0, sizeof(solution->stat));
        unsigned int result;
        result = solve1(msg, map, param, verbose, NULL, solution);
#if !defined(SMALL_NODE)
        while (result == SOLVE_SOLVABLE) {
                {
                        struct node *n = SLIST_FIRST(&solution->moves);
                        printf("step %u (the first known state):\n", n->steps);
                        dump_map(n->map);
                }
                {
                        struct node *n =
                                SLIST_LAST(&solution->moves, struct node, q);
                        printf("step %u (the final state):\n", n->steps);
                        dump_map(n->map);
                }

                detach_solution(solution);
                solve_cleanup();
                struct node *n = SLIST_FIRST(&solution->moves);
                assert(n->steps > 1);
                struct solution solution2;
                map_t goal;
                prev_map(n, n->map, goal);
                printf("re-solving up to the intermediate goal (step %u):\n",
                       n->steps - 1);
                dump_map(goal);
                result = solve1(msg, map, param, false, goal, &solution2);
                if (result != SOLVE_SOLVED && result != SOLVE_SOLVABLE) {
                        printf("solve() returned an unexpected result %u\n",
                               result);
                        exit(1); /* must be a bug */
                }
                assert(SLIST_FIRST(&solution->moves)->steps ==
                       SLIST_LAST(&solution2.moves, struct node, q)->steps +
                               1);
                detach_solution(&solution2);
                solve_cleanup();
                assert(solution->detached == solution2.detached);
                SLIST_SPLICE_HEAD(&solution->moves, &solution2.moves, q);
        }
#endif
        assert(result != SOLVE_SOLVED ||
               SLIST_FIRST(&solution->moves)->steps == 1);
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
#if defined(SMALL_NODE)
                for (n = SLIST_FIRST(h); n != NULL; n = next) {
#else
                for (n = LIST_FIRST(h); n != NULL; n = next) {
#endif
#if defined(SMALL_NODE)
                        next = SLIST_NEXT(n, hashq);
#else
                        next = LIST_NEXT(n, hashq);
#endif
                        free_node(n);
                }
        }
#endif
#if defined(USE_BLOOM_FILTER)
        memset(filter, 0, sizeof(filter));
        memset(&bloom_filter_stats, 0, sizeof(bloom_filter_stats));
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
        struct node_slist h;
        SLIST_HEAD_INIT(&h);
        struct node *n;
        SLIST_FOREACH(n, &solution->moves, q) {
                struct node *nn = malloc(sizeof(*nn));
                *nn = *n;
                nn->parent = NULL;
                SLIST_INSERT_TAIL(&h, nn, q);
        }
        SLIST_HEAD_INIT(&solution->moves);
        SLIST_SPLICE_TAIL(&solution->moves, &h, q);
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
        while ((n = SLIST_FIRST(&solution->moves)) != NULL) {
                assert(n->parent == NULL);
                SLIST_REMOVE(&solution->moves, (struct node *)NULL, n, q);
                free(n);
        }
}

#if !defined(MEMLIMIT_GB)
#define MEMLIMIT_GB 8
#endif
#if !defined(MAX_ITERATIONS)
#define MAX_ITERATIONS 1000000000
#endif

struct solver_param solver_default_param = {
        .limit = (size_t)MEMLIMIT_GB * 1024 * 1024 * 1024,
        .max_iterations = MAX_ITERATIONS,
};
