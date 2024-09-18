#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
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

LIST_HEAD_NAMED(struct node, hash_head) hash_heads[HASH_SIZE];
LIST_HEAD(struct node) todo;

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
        struct hash_head *head = &hash_heads[idx];
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

char
objchr(uint8_t objidx)
{
        return "_WBXLDRUPA"[objidx];
}

void
evaluate(struct node *n)
{
        unsigned int nswitch = 0;
        unsigned int npush = 0;
        unsigned int npush_cont = 0;
        unsigned int npush_sameobj = 0;
        loc_t last_pushed_obj_loc = -1;
        LIST_HEAD(struct node) h;
        LIST_HEAD_INIT(&h);
        do {
                LIST_INSERT_HEAD(&h, n, q);
                n = n->parent;
        } while (n->parent != NULL);
        struct node *prev = NULL;
        LIST_FOREACH(n, &h, q)
        {
                bool same = false;
                if (prev != NULL && next_loc(prev) == n->loc) {
                        same = true;
                }
                int chr;
                uint8_t objidx = n->map[next_loc(n)];
                if (objidx == A) {
                        chr = 'A';
                } else {
                        chr = 'P';
                }
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
                }
                if ((n->flags & MOVE_GET_BOMB) != 0) {
                        printf(" GET_BOMB");
                }
                if ((n->flags & MOVE_BEAM) != 0) {
                        printf(" BEAM");
                }
                printf("\n");
                if ((n->flags & MOVE_PUSH) != 0) {
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
                        last_pushed_obj_loc = -1;
                }
                prev = n;
        }
        printf("nswitch %u\n", nswitch);
        printf("npush %u\n", npush);
        printf("npush_cont %u\n", npush_cont);
        printf("npush_sameobj %u\n", npush_sameobj);
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
        unsigned int queued = 0;
        unsigned int processed = 0;
        unsigned int duplicated = 0;
        struct node *n = alloc_node();
        struct map_info info;
        printf("stage %u\n", stage_number);
        decode_stage(stage_number - 1, n->map, &info);
        if (info.w > width || info.h > height) {
                printf("info %u %u\n", info.w, info.h);
                printf("macro %u %u\n", width, height);
                exit(1);
        }
        LIST_INSERT_TAIL(&todo, n, q);
        n->parent = NULL;
        n->steps = 0;
        queued++;
        add(n);
        while ((n = LIST_FIRST(&todo)) != NULL) {
                LIST_REMOVE(&todo, n, q);
                struct stage_meta meta;
                map_t beam_map;
                calc_stage_meta(n->map, &meta);
                calc_beam(n->map, beam_map);
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
                                        evaluate(n2);
                                        dump_hash();
                                        exit(0);
                                }
                        }
                }
                processed++;
                if ((processed % 100000) == 0) {
                        printf("processed %u / %u (%f) duplicated %u (%f) "
                               "step %u\n",
                               processed, queued, (float)queued / processed,
                               duplicated, (float)duplicated / processed,
                               n->steps);
                        // dump_hash();
                }
        }
        printf("impossible\n");
}