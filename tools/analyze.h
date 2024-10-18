#include <stdbool.h>

#include "defs.h"
#include "rule.h"

/*
 * reachability values
 */
#define REACHABLE 1
#define REACHABLE_WALL 2 /* an unmovable object acting as a wall */
#define UNREACHABLE 0

/*
 * "movable" values
 *
 * UNMOVABLE doesn't allow false positives.
 */
#define NEEDVISIT 0x10 /* internal use */
#define PUSHABLE 0x01
#define COLLECTABLE 0x02

#define is_UNMOVABLE(x) (((x) & (PUSHABLE | COLLECTABLE)) == 0)

void calc_reachable_from(const map_t map, const map_t movable, loc_t loc,
                         map_t reachable);
bool calc_reachable_from_A(const map_t map, const map_t movable,
                           map_t reachable);
void calc_reachable_from_any_A(const map_t map, const map_t movable,
                               const map_t beam, map_t reachable);
void calc_movable(const map_t map, bool do_collect, map_t movable);
void calc_surrounded(const map_t map, const map_t movable, map_t surrounded);
bool is_simple_movable_object(uint8_t objidx);
bool permanently_blocked(const map_t map, const map_t movable, loc_t loc);
bool tsumi(const map_t map);
