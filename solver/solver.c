#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "hash.h"
#include "list.h"
#include "rule.h"
#include "node.h"
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
        LIST_FOREACH(n2, head, hashq)
        {
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
                LIST_FOREACH(dn, &hash_heads[i], hashq) { n++; }
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
solve(struct node *root, unsigned int max_iterations,
      struct node_list *solution)
{
        LIST_HEAD_INIT(&todo);
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                LIST_HEAD_INIT(&hash_heads[i]);
        }

        unsigned int queued = 0;
        unsigned int processed = 0;
        unsigned int duplicated = 0;

        root->parent = NULL;
        root->steps = 0;
        LIST_INSERT_TAIL(&todo, root, q);
        queued++;
        add(root);

        struct node *n;
        while ((n = LIST_FIRST(&todo)) != NULL) {
                LIST_REMOVE(&todo, n, q);
                struct stage_meta meta;
                map_t beam_map;
                calc_stage_meta(n->map, &meta);
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
                                memcpy(n2->map, n->map, sizeof(n->map));
                                struct stage_meta meta2 = meta;
                                player_move(&meta2, &meta2.players[i], dir,
                                            n2->map, beam_map, true);
                                if (add(n2)) {
                                        duplicated++;
                                        free(n2);
                                        continue;
                                }
                                n2->parent = n;
                                n2->steps = n->steps + 1;
                                n2->loc = p->loc;
                                n2->dir = dir;
                                n2->flags = flags;
                                LIST_INSERT_TAIL(&todo, n2, q);
                                queued++;
                                if (meta2.nbombs == 0) {
                                        printf("solved!\n");
                                        return_solution(n2, solution);
                                        // dump_hash();
                                        return SOLVE_SOLVED;
                                }
                        }
                }
                processed++;
                if ((processed % 100000) == 0) {
                        printf("processed %u / %u (%u) dup %u (%.3f) "
                               "step %u\n",
                               processed, queued, queued - processed,
                               duplicated, (float)duplicated / processed,
                               n->steps);
                        // dump_hash();
                }
                if (processed >= max_iterations) {
                        printf("giving up\n");
                        return SOLVE_GIVENUP;
                }
        }
        printf("impossible\n");
        return SOLVE_IMPOSSIBLE;
}

void
solve_cleanup(void)
{
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                struct node_list *h = &hash_heads[i];
                struct node *n;
                while ((n = LIST_FIRST(h)) != NULL) {
                        LIST_REMOVE(h, n, hashq);
                        free(n);
                }
        }
}

