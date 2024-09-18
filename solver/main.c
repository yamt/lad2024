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

#define HASH_SIZE 1024021

struct node {
        map_t map;
        LIST_ENTRY(struct node) q;
        LIST_ENTRY(struct node) hashq;
        struct node *parent;
        unsigned int steps;

        /* move from the parent. note: this is redundant. */
        loc_t loc;
        enum diridx dir;
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
        LIST_INSERT_TAIL(head, n, hashq);
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
dump(const struct node *n)
{
        do {
                printf("step %3u: x=%u y=%u dir=%c\n", n->steps, loc_x(n->loc),
                       loc_y(n->loc), "LDRU"[n->dir]);
                n = n->parent;
        } while (n->parent != NULL);
}

int
main(int argc, char **argv)
{
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
        decode_stage(12, n->map, &info);
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
                                LIST_INSERT_TAIL(&todo, n2, q);
                                queued++;
                                if (meta2.nbombs == 0) {
                                        printf("solved!\n");
                                        dump(n2);
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
                        dump_hash();
                }
        }
        printf("impossible\n");
}
