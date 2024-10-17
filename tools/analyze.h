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
#define UNVISITED 0 /* internal use */
#define VISITED 1   /* internal use */
#define MOVABLE 2   /* movable (or collectable for bombs) */
#define UNMOVABLE 3 /* impossible to move (or collect) */
#define PUSHABLE 0x10
#define COLLECTABLE 0x20

#define is_MOVABLE(x) (((x) & 0xf) == MOVABLE)
#define is_UNMOVABLE(x) (((x) & 0xf) == UNMOVABLE)

void calc_reachable_from(const map_t map, const map_t movable, loc_t loc,
                         map_t reachable);
bool calc_reachable_from_A(const map_t map, const map_t movable,
                           map_t reachable);
void calc_reachable_from_any_A(const map_t map, const map_t movable,
                               map_t reachable);
void calc_movable(const map_t map, map_t movable);
void calc_surrounded(const map_t map, const map_t movable, map_t surrounded);
bool is_simple_movable_object(uint8_t objidx);
bool permanently_blocked(const map_t map, const map_t movable, loc_t loc);
bool tsumi(const map_t map);
