#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "hash.h"
#include "list.h"
#include "loader.h"
#include "rule.h"

// #define HASH_SIZE 1024021
#define HASH_SIZE 10240033

struct node {
        map_t map;
        LIST_ENTRY(struct node) q;
        LIST_ENTRY(struct node) hashq;
        struct node *parent;
        unsigned int steps;

        /* move from the parent. note: this is redundant. */
        loc_t loc;
        enum diridx dir;
        unsigned int flags;
};

LIST_HEAD_NAMED(struct node, node_list);

struct node_list hash_heads[HASH_SIZE];
struct node_list todo;

struct node *
alloc_node(void)
{
        struct node *n = malloc(sizeof(*n));
        if (n == NULL) {
                exit(1);
        }
        return n;
}

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

loc_t
next_loc(struct node *n)
{
        return n->loc + dirs[n->dir].loc_diff;
}

loc_t
pushed_obj_loc(struct node *n)
{
        return n->loc + dirs[n->dir].loc_diff * 2;
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
evaluate(struct node_list *solution)
{
        unsigned int nswitch = 0;
        unsigned int npush = 0;
        unsigned int npush_cont = 0;
        unsigned int npush_sameobj = 0;
        unsigned int nbeam_changed = 0;
        unsigned int nsuicide = 0;
        loc_t last_pushed_obj_loc = -1;
        struct node *n;
        struct node *prev = NULL;
        LIST_FOREACH(n, solution, q)
        {
                map_t beam_map;
                bool same = false;
                if (prev != NULL && next_loc(prev) == n->loc) {
                        same = true;
                }
                const uint8_t objidx = n->map[next_loc(n)];
                const bool is_robot = objidx == A;
                const int chr = objchr(objidx);
                if (same) {
                        printf("step %3u:               dir=%c", n->steps,
                               "LDRU"[n->dir]);
                } else {
                        printf("step %3u: %c (x=%2u y=%2u) dir=%c", n->steps,
                               chr, loc_x(n->loc), loc_y(n->loc),
                               "LDRU"[n->dir]);
                        nswitch++;
                }
                if ((n->flags & MOVE_PUSH) != 0) {
                        printf(" PUSH(%c)", objchr(n->map[pushed_obj_loc(n)]));
                        loc_t obj_loc_before_push = next_loc(n);
                        if (prev != NULL && (prev->flags & MOVE_PUSH) != 0 &&
                            prev->dir == n->dir) {
                                npush_cont++;
                        }
                        if (obj_loc_before_push == last_pushed_obj_loc) {
                                npush_sameobj++;
                        }
                        npush++;
                        last_pushed_obj_loc = pushed_obj_loc(n);
                }
                if ((n->flags & MOVE_GET_BOMB) != 0) {
                        printf(" GET_BOMB");
                        last_pushed_obj_loc = -1;
                }
                if ((n->flags & MOVE_BEAM) != 0) {
                        printf(" BEAM");
                        nbeam_changed++;
                }
                calc_beam(n->map, beam_map);
                if (is_robot != (beam_map[next_loc(n)] != 0)) {
                        nsuicide++;
                        printf(" SUICIDAL");
                }
                printf("\n");
                prev = n;
        }
        printf("nswitch %u\n", nswitch);
        printf("npush %u\n", npush);
        printf("npush_cont %u\n", npush_cont);
        printf("npush_sameobj %u\n", npush_sameobj);
        printf("nbeam_changed %u\n", nbeam_changed);
        printf("nsuicide %u\n", nsuicide);
        unsigned int score = nswitch * 2 + npush * 2 - npush_cont -
                             npush_sameobj + nsuicide;
        printf("score %u\n", score);
        return score;
}

#define SOLVE_SOLVED 0x01
#define SOLVE_IMPOSSIBLE 0x02
#define SOLVE_GIVENUP 0x03

unsigned int
solve(struct node *root, struct node_list *solution)
{
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

int
main(int argc, char **argv)
{
        if (argc != 2) {
                exit(2);
        }
        int stage_number = atoi(argv[1]);
        if (stage_number <= 0) {
                exit(2);
        }
        LIST_HEAD_INIT(&todo);
        unsigned int i;
        for (i = 0; i < HASH_SIZE; i++) {
                LIST_HEAD_INIT(&hash_heads[i]);
        }
        struct node *n = alloc_node();
        struct map_info info;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, n->map, &info);
        if (info.w > width || info.h > height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", width, height);
                exit(1);
        }
        dump_map(n->map);
        struct node_list solution;
        unsigned int result = solve(n, &solution);
        if (result == SOLVE_SOLVED) {
                unsigned int score = evaluate(&solution);
                printf("score %u\n", score);
        }
        solve_cleanup();
}
