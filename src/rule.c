#include "rule.h"
#include "defs.h"

const struct dir dirs[] = {
        [LEFT] =
                {
                        -1,
                        0,
                },
        [DOWN] =
                {
                        0,
                        1,
                },
        [RIGHT] =
                {
                        1,
                        0,
                },
        [UP] =
                {
                        0,
                        -1,
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
in_map(int x, int y)
{
        return 0 <= x && x < width && 0 <= y && y < height;
}

void
calc_beam(const map_t map, map_t beam_map)
{
        int x;
        int y;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        beam_map[y][x] = 0;
                }
        }
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        uint8_t objidx = map[y][x];
                        if (!is_light(objidx)) {
                                continue;
                        }
                        const struct dir *dir = &dirs[light_dir(objidx)];
                        int bx = x;
                        int by = y;
                        while (1) {
                                bx += dir->dx;
                                by += dir->dy;
                                if (!in_map(bx, by) ||
                                    block_beam(map[by][bx])) {
                                        break;
                                }
                                beam_map[by][bx] = 1;
                        }
                }
        }
}
