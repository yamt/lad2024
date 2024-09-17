#include <stdbool.h>
#include <stdint.h>

#define width 20
#define height 20

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

bool is_light(uint8_t objidx);
bool is_player(uint8_t objidx);
bool is_bomb(uint8_t objidx);
bool can_push(uint8_t objidx);
int light_dir(uint8_t objidx);
bool block_beam(uint8_t objidx);
bool in_map(loc_t loc);
void calc_beam(const map_t map, map_t beam_map);
