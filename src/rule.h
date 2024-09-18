#if !defined(_LAD_RULE_H)
#define _LAD_RULE_H

#include <stdbool.h>
#include <stdint.h>

#define width 20
#define height 20
#define max_players 4

typedef uint8_t map_t[height * width];

enum diridx {
        NONE = -1,
        LEFT = 0,
        DOWN = 1,
        RIGHT = 2,
        UP = 3,
};

typedef int loc_t;
#define loc_x(loc) ((loc) % width)
#define loc_y(loc) ((loc) / width)
#define genloc(x, y) ((x) + (y) * width)

struct dir {
        loc_t loc_diff;
};

extern const struct dir dirs[4];

struct stage_meta {
        int nplayers;
        struct player {
                loc_t loc;
        } players[max_players];
        int nbombs;

        int stage_height;
};

bool is_light(uint8_t objidx);
bool is_player(uint8_t objidx);
bool is_bomb(uint8_t objidx);
bool can_push(uint8_t objidx);
int light_dir(uint8_t objidx);
bool block_beam(uint8_t objidx);
bool in_map(loc_t loc);
void calc_beam(const map_t map, map_t beam_map);
void move_object(map_t map, loc_t nloc, loc_t oloc);
struct player *player_at(struct stage_meta *meta, loc_t loc);

#define MOVE_OK 0x01
#define MOVE_PUSH 0x02     /* push something */
#define MOVE_BEAM 0x04     /* might need to recalculate beam */
#define MOVE_GET_BOMB 0x08 /* got a bomb */

unsigned int player_move(struct stage_meta *meta, struct player *p,
                         enum diridx dir, map_t map, const map_t beam_map,
                         bool commit);

#endif /* !defined(_LAD_RULE_H) */
