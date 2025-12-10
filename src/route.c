#include <string.h>

#include "defs.h"
#include "route.h"
#include "rule.h"
#include "util.h"
#include "wasm4.h"

/* 2 * max(map_width, map_height) is enough */
#define MAX_SEARCH_WIDTH 40

static unsigned int cidx;
static unsigned int pidx;
static struct queue {
        loc_t loc;
} queue[MAX_SEARCH_WIDTH];

static void
visit(const map_t map, const map_t beam, loc_t nloc, loc_t from, bool is_A,
      map_t route, enum diridx dir)
{
        ASSERT(dir != NONE);
        if (nloc < 0 || map_width * map_height <= nloc) {
                return;
        }
        if ((enum diridx)route[nloc] != NONE) {
                return;
        }
        if (beam[nloc] != is_A) {
                return;
        }
        if (nloc != from && map[nloc] != _) {
                return;
        }
        struct queue *nq = &queue[pidx];
        nq->loc = nloc;
        ASSERT((enum diridx)route[nloc] == NONE);
        route[nloc] = (uint8_t)dir;
        pidx = (pidx + 1) % MAX_SEARCH_WIDTH;
        ASSERT(pidx != cidx);
}

void
route_calculate(const map_t map, const map_t beam, loc_t to, loc_t from,
                bool is_A, map_t route)
{
        ASSERT(pidx == cidx);
        memset(route, NONE, map_width * map_height);
        if (to == from || beam[to] != is_A || beam[from] != is_A) {
                return;
        }
        visit(map, beam, to, from, is_A, route, HERE);
        while (cidx != pidx) {
                const struct queue *q = &queue[cidx];
                cidx = (cidx + 1) % MAX_SEARCH_WIDTH;
                enum diridx i;
                for (i = 0; i < 4; i++) {
                        loc_t nloc = q->loc - dir_loc_diff(i);
                        visit(map, beam, nloc, from, is_A, route, i);
                }
        }
        ASSERT((enum diridx)route[to] == HERE ||
               (enum diridx)route[to] == NONE);
        route[to] = (uint8_t)NONE;
        ASSERT(pidx == cidx);
}
