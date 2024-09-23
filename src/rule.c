#include <stddef.h>

#include "defs.h"
#include "rule.h"

const struct dir dirs[] = {
        [LEFT] =
                {
                        -1,
                },
        [DOWN] =
                {
                        map_width,
                },
        [RIGHT] =
                {
                        1,
                },
        [UP] =
                {
                        -map_width,
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
        return 0 <= loc && loc < map_width * map_height;
}

void
calc_beam(const map_t map, map_t beam_map)
{
        loc_t loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                beam_map[loc] = 0;
        }
        for (loc = 0; loc < map_width * map_height; loc++) {
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

void
move_object(map_t map, loc_t nloc, loc_t oloc)
{
        map[nloc] = map[oloc];
        map[oloc] = _;
}

struct player *
player_at(struct stage_meta *meta, loc_t loc)
{
        int i;
        for (i = 0; i < meta->nplayers; i++) {
                struct player *p = &meta->players[i];
                if (p->loc == loc) {
                        return p;
                }
        }
        return NULL;
}

void
calc_stage_meta(map_t map, struct stage_meta *meta)
{
        int nplayers = 0;
        int nbombs = 0;
        int loc;
        for (loc = 0; loc < map_width * map_height; loc++) {
                uint8_t objidx = map[loc];
                if (is_player(objidx)) {
                        struct player *p = &meta->players[nplayers++];
                        p->loc = loc;
                } else if (is_bomb(objidx)) {
                        nbombs++;
                }
        }
        meta->nplayers = nplayers;
        meta->nbombs = nbombs;
}

unsigned int
player_move(struct stage_meta *meta, struct player *p, enum diridx dir,
            map_t map, const map_t beam_map, bool commit)
{
        bool is_robot = map[p->loc] == A;
        const struct dir *d = &dirs[dir];
        int loc_diff = d->loc_diff;
        if ((beam_map[p->loc] != 0) != is_robot) {
                return 0;
        }
        int loc = p->loc + loc_diff;
        if (!in_map(loc)) {
                return 0;
        }
        unsigned int flags = 0;
        uint8_t objidx = map[loc];
        if (is_robot && is_bomb(objidx)) {
                flags |= MOVE_OK | MOVE_BEAM | MOVE_GET_BOMB;
                if (commit) {
                        meta->nbombs--;
                }
        } else if (objidx == _) {
                flags |= MOVE_OK;
        } else if (can_push(objidx)) {
                int nloc = loc + loc_diff;
                if (in_map(nloc) && map[nloc] == _) {
                        flags |= MOVE_OK | MOVE_PUSH;
                        if (block_beam(objidx)) {
                                flags |= MOVE_BEAM;
                        }
                        if (commit) {
                                if (is_player(objidx)) {
                                        struct player *p2 =
                                                player_at(meta, loc);
                                        p2->loc = nloc;
                                }
                                move_object(map, nloc, loc);
                        }
                }
        }
        if ((flags & MOVE_OK) != 0 && commit) {
                move_object(map, loc, p->loc);
                p->loc = loc;
        }
        return flags;
}
