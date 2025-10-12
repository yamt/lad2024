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

static int
noop_printf(const char *fmt, ...)
{
        return 0;
}

void
evaluate(const map_t map, const struct node_slist *solution, bool verbose,
         struct evaluation *ev)
{
        int (*printf_fn)(const char *fmt, ...) =
                verbose ? printf : noop_printf;
        unsigned int nswitch = 0;
        unsigned int npush = 0;
        unsigned int npush_cont = 0;
        unsigned int npush_sameobj = 0;
        unsigned int nbeam_changed = 0;
        unsigned int nsuicide = 0;
        loc_t last_pushed_obj_loc = -1;
        struct node *n;
        struct node *prev = NULL;
        SLIST_FOREACH(n, solution, q) {
                map_t beam_map;
                bool same = false;
                if (prev != NULL && next_loc(prev) == n->loc) {
                        same = true;
                }
#if defined(SMALL_NODE)
                map_t n_map;
                node_expand_map(n, map, n_map);
#else
                const uint8_t *n_map = n->map;
#endif
                const uint8_t objidx = n_map[next_loc(n)];
                const bool is_robot = objidx == A;
                const int chr = objchr(objidx);
                if (same) {
                        printf_fn("step %3u:               dir=%c", n->steps,
                                  dirchr(n->dir));
                } else {
                        printf_fn("step %3u: %c (x=%2u y=%2u) dir=%c",
                                  n->steps, chr, loc_x(n->loc), loc_y(n->loc),
                                  dirchr(n->dir));
                        nswitch++;
                }
                if ((n->flags & MOVE_PUSH) != 0) {
                        printf_fn(" PUSH(%c)",
                                  objchr(n_map[pushed_obj_loc(n)]));
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
                        printf_fn(" GET_BOMB");
                        last_pushed_obj_loc = -1;
                }
                if ((n->flags & MOVE_BEAM) != 0) {
                        printf_fn(" BEAM");
                        nbeam_changed++;
                }
                calc_beam(n_map, beam_map);
                if (is_robot != (beam_map[next_loc(n)] != 0)) {
                        nsuicide++;
                        printf_fn(" SUICIDAL");
                }
                printf_fn("\n");
                prev = n;
        }
        printf_fn("nswitch %u\n", nswitch);
        printf_fn("npush %u\n", npush);
        printf_fn("npush_cont %u\n", npush_cont);
        printf_fn("npush_sameobj %u\n", npush_sameobj);
        printf_fn("nbeam_changed %u\n", nbeam_changed);
        printf_fn("nsuicide %u\n", nsuicide);
        map_t simplified_map;
        map_copy(simplified_map, map);
        simplify(simplified_map);
        struct size size;
        measure_size(simplified_map, &size);
        unsigned int map_area =
                (size.xmax - size.xmin + 1) * (size.ymax - size.ymin + 1);
        printf_fn("map size %u\n", map_area);
        unsigned int score = nswitch * 2 + npush * 2 - npush_cont -
                             npush_sameobj + nsuicide;
        score = score * 20 * 20 / map_area;
        printf_fn("score %u\n", score);
        ev->score = score;
}
