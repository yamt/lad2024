#include "rule.h"
#include "defs.h"

const struct dir dirs[] = {
        [LEFT] =
                {
                        -1,
                },
        [DOWN] =
                {
                        width,
                },
        [RIGHT] =
                {
                        1,
                },
        [UP] =
                {
                        -width,
                },
};

bool
is_light(uint8_t objidx)
{
        return objidx == U || objidx == R || objidx == D || objidx == L;
}

bool
is_player(uint8_t objidx)
{
        return objidx == P || objidx == A;
}

bool
is_bomb(uint8_t objidx)
{
        return objidx == X;
}

bool
can_push(uint8_t objidx)
{
        return is_light(objidx) || is_player(objidx) || is_bomb(objidx) ||
               objidx == B;
}

int
light_dir(uint8_t objidx)
{
        return objidx - L;
}

bool
block_beam(uint8_t objidx)
{
        return is_light(objidx) || objidx == W || objidx == X || objidx == B;
}

bool
in_map(loc_t loc)
{
        return 0 <= loc && loc < width * height;
}

void
calc_beam(const map_t map, map_t beam_map)
{
        loc_t loc;
        for (loc = 0; loc < width * height; loc++) {
                beam_map[loc] = 0;
        }
        for (loc = 0; loc < width * height; loc++) {
                uint8_t objidx = map[loc];
                if (!is_light(objidx)) {
                        continue;
                }
                const struct dir *dir = &dirs[light_dir(objidx)];
                loc_t bloc = loc;
                while (1) {
                        bloc += dir->loc_diff;
                        if (!in_map(bloc) || block_beam(map[bloc])) {
                                break;
                        }
                        beam_map[bloc] = 1;
                }
        }
}
