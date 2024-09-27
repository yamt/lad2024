#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "dump.h"
#include "evaluater.h"
#include "hash.h"
#include "maputil.h"
#include "node.h"
#include "rule.h"
#include "simplify.h"

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
evaluate(const struct node *root, const struct node_list *solution,
         struct evaluation *ev)
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
        LIST_FOREACH(n, solution, q) {
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
        map_t simplified_map;
        memcpy(simplified_map, root->map, map_width * map_height);
        simplify(simplified_map);
        struct size size;
        measure_size(simplified_map, &size);
        unsigned int map_area =
                (size.xmax - size.xmin + 1) * (size.ymax - size.ymin + 1);
        printf("map size %u\n", map_area);
        unsigned int score = nswitch * 2 + npush * 2 - npush_cont -
                             npush_sameobj + nsuicide;
        score = score * 20 * 20 / map_area;
        printf("score %u\n", score);
        ev->score = score;
}
