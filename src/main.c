#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "wasm4.h"

#include "defs.h"

#include "rule.h"

#include "loader.h"

/* clang-format off */

const uint8_t person[] = {
        0b00011000,
        0b10011001,
        0b01111110,
        0b00111100,
        0b00111100,
        0b00111100,
        0b00100100,
        0b00100100,

        0b00011000,
        0b00011000,
        0b11111111,
        0b00111100,
        0b00111100,
        0b00111100,
        0b00100100,
        0b00100100,

        0b00011000,
        0b00011000,
        0b01111110,
        0b10111101,
        0b00111100,
        0b00111100,
        0b00100100,
        0b00100100,
};

const uint8_t robot[] = {
        0b00000100,
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b01111110,
        0b11111111,
        0b01111110,

        0b00001000,
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b11111111,
        0b01111110,
        0b01111110,

        0b00010000,
        0b00111100,
        0b01011010,
        0b01111110,
        0b11111111,
        0b01111110,
        0b01111110,
        0b01111110,
};

const uint8_t box[] = {
        0b00000000,
        0b01110110,
        0b11110111,
        0b10001001,
        0b11110111,
        0b11110111,
        0b01110110,
        0b00000000,
};

const uint8_t bomb[] = {
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b01111110,
        0b01111110,
        0b01011010,
        0b00111100,
};

const uint8_t light[] = {
        0b11111000,
        0b01111110,
        0b11111111,
        0b01111111,
        0b11111111,
        0b01111111,
        0b11111110,
        0b11111000,
};

const uint8_t wall[] = {
        0b01111110,
        0b11111111,
        0b10000001,
        0b11111111,
        0b11111111,
        0b10000001,
        0b11111111,
        0b01111110,
};

/* clang-format on */

const struct obj {
        const uint8_t *sprite;
        uint16_t color;
        uint8_t flags;
} objs[] = {
        [_] =
                {
                        .sprite = NULL,
                },
        [W] =
                {
                        .sprite = wall,
                        .color = 0x40,
                        .flags = 0,
                },
        [U] =
                {
                        .sprite = light,
                        .color = 0x20,
                        .flags = BLIT_FLIP_X | BLIT_FLIP_Y | BLIT_ROTATE,
                },
        [R] =
                {
                        .sprite = light,
                        .color = 0x20,
                        .flags = BLIT_FLIP_X | BLIT_FLIP_Y,
                },
        [D] =
                {
                        .sprite = light,
                        .color = 0x20,
                        .flags = BLIT_ROTATE,
                },
        [L] =
                {
                        .sprite = light,
                        .color = 0x20,
                        .flags = 0,
                },
        [B] =
                {
                        .sprite = box,
                        .color = 0x20,
                        .flags = 0,
                },
        [X] =
                {
                        .sprite = bomb,
                        .color = 0x20,
                        .flags = 0,
                },
        [P] =
                {
                        .sprite = person,
                        .color = 0x40,
                        .flags = 0,
                },
        [A] =
                {
                        .sprite = robot,
                        .color = 0x40,
                        .flags = 0,
                },
};

#define ASSERT(cond)                                                          \
        do {                                                                  \
                if (!(cond)) {                                                \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)

#define max_players 4

static uint8_t prev_gamepad = 0;
static unsigned int frame = 0;

#define moving_nsteps 4
static unsigned int beamidx = 0;
static unsigned int moving_step = 0;
static enum diridx moving_dir = 0;
static bool moving_pushing = false;
static bool moving_beam = false;

#define CALC_BEAM 1U
#define MESSAGE 4U
#define ALL 7U
static unsigned int need_redraw;
static struct redraw_rect {
        int xmin;
        int ymin;
        int xmax;
        int ymax;
} redraw_rect;

struct stage_meta {
        int nplayers;
        int cur;
        struct player {
                loc_t loc;
        } players[max_players];
        int nbombs;

        int stage_height;
        const char *message;
} meta;

#define max_stages 100
#define save_data_version 1
struct save_data {
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t clear_bitmap[(max_stages + 32 - 1) / 32];
} state;

map_t map;
map_t beam[2];

bool
is_cur_player(loc_t loc)
{
        const struct player *p = &meta.players[meta.cur];
        return p->loc == loc;
}

struct player *
cur_player()
{
        return &meta.players[meta.cur];
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
switch_player()
{
        meta.cur = (meta.cur + 1) % meta.nplayers;
}

bool
is_moving(loc_t loc)
{
        if (moving_step == 0) {
                return false;
        }
        if (is_cur_player(loc)) {
                return true;
        }
        if (!moving_pushing) {
                return false;
        }
        const struct dir *d = &dirs[moving_dir];
        return is_cur_player(loc - d->loc_diff);
}

void
mark_redraw_object(loc_t loc)
{
        int x = loc_x(loc);
        int y = loc_y(loc);
        if (redraw_rect.xmin > x) {
                redraw_rect.xmin = x;
        }
        if (redraw_rect.xmax <= x) {
                redraw_rect.xmax = x + 1;
        }
        if (redraw_rect.ymin > y) {
                redraw_rect.ymin = y;
        }
        if (redraw_rect.ymax <= y) {
                redraw_rect.ymax = y + 1;
        }
}

void
mark_redraw_cur_player()
{
        struct player *p = cur_player();
        mark_redraw_object(p->loc);
}

void
mark_redraw_all_objects()
{
        /* XXX this assumes how mark_redraw_object is implemented */
        mark_redraw_object(0);
        mark_redraw_object(width * meta.stage_height - 1);
}

void
mark_redraw_all()
{
        need_redraw = ALL;
        mark_redraw_all_objects();
        redraw_rect.ymax = height;
}

void
move_object(loc_t nloc, loc_t oloc)
{
        mark_redraw_object(oloc);
        mark_redraw_object(nloc);
        map[nloc] = map[oloc];
        map[oloc] = _;
}

void
update_beam()
{
        beamidx = 1 - beamidx;
        calc_beam(map, beam[beamidx]);
        mark_redraw_all_objects();
}

void
draw_beam()
{
        unsigned int curidx = beamidx;
        unsigned int altidx = beamidx;
        bool horizontal;
        int dx = 0;
        int dy = 0;
        if (moving_step && moving_beam) {
                ASSERT(moving_dir != NONE);
                const struct dir *d = &dirs[moving_dir];
                int prop;
                int loc_diff = d->loc_diff;
                horizontal = loc_diff == -1 || loc_diff == 1;
                if (loc_diff < 0) {
                        prop = 8 * (moving_nsteps - moving_step) /
                               moving_nsteps;
                        loc_diff = -loc_diff;
                        curidx = 1 - curidx;
                } else {
                        prop = 8 * moving_step / moving_nsteps;
                        altidx = 1 - curidx;
                }
                loc_diff = loc_diff * prop;
                dx = loc_x(loc_diff);
                dy = loc_y(loc_diff);
        }
        int x;
        int y;
        for (y = redraw_rect.ymin; y < redraw_rect.ymax; y++) {
                for (x = redraw_rect.xmin; x < redraw_rect.xmax; x++) {
                        loc_t bloc = genloc(x, y);
                        uint8_t cur = beam[curidx][bloc];
                        uint8_t alt = beam[altidx][bloc];
                        if (cur == alt) {
                                if (cur) {
                                        *DRAW_COLORS = 3;
                                } else {
                                        *DRAW_COLORS = 1;
                                }
                                rect(x * 8, y * 8, 8, 8);
                        } else {
                                if (alt) {
                                        *DRAW_COLORS = 3;
                                } else {
                                        *DRAW_COLORS = 1;
                                }
                                if (horizontal) {
                                        rect(x * 8 + dx, y * 8,
                                             (uint32_t)(8 - dx), 8);
                                } else {
                                        rect(x * 8, y * 8 + dy, 8,
                                             (uint32_t)(8 - dy));
                                }
                                if (cur) {
                                        *DRAW_COLORS = 3;
                                } else {
                                        *DRAW_COLORS = 1;
                                }
                                if (horizontal) {
                                        rect(x * 8, y * 8, (uint32_t)dx, 8);
                                } else {
                                        rect(x * 8, y * 8, 8, (uint32_t)dy);
                                }
                        }
                }
        }
}

void
draw_object(int x, int y, uint8_t objidx)
{
        const struct obj *obj = &objs[objidx];
        *DRAW_COLORS = obj->color;
        loc_t loc = x + y * width;
        int i = 0;
        int dx = 0;
        int dy = 0;
        if (is_player(objidx) && is_cur_player(loc)) {
                i = (frame / 8) % 3;
        }
        if ((is_player(objidx) || can_push(objidx)) && is_moving(loc)) {
                const struct dir *dir = &dirs[moving_dir];
                int prop = 8 * (moving_nsteps - moving_step) / moving_nsteps;
                int loc_diff = -dir->loc_diff * prop;
                dx = loc_x(loc_diff);
                dy = loc_y(loc_diff);
        }
        blit(obj->sprite + i * 8, x * 8 + dx, y * 8 + dy, 8, 8, obj->flags);
}

void
draw_objects()
{
        int x;
        int y;
        for (y = redraw_rect.ymin; y < redraw_rect.ymax; y++) {
                for (x = redraw_rect.xmin; x < redraw_rect.xmax; x++) {
                        uint8_t objidx = map[genloc(x, y)];
                        draw_object(x, y, objidx);
                }
        }
}

void
digits(unsigned int v, int x, int y)
{
        char buf[4];
        buf[0] = '0' + ((v / 100) % 10);
        buf[1] = '0' + ((v / 10) % 10);
        buf[2] = '0' + (v % 10);
        buf[3] = 0;
        text(buf, x, y);
}

void
draw_message()
{
        *DRAW_COLORS = 0x04;
        text("STAGE     (   %)", (20 - 7 - 3 - 6) * 8, 19 * 8);
        if ((state.clear_bitmap[state.cur_stage / 32] &
             1 << (state.cur_stage % 32)) == 0) {
                *DRAW_COLORS = 0x02;
        }
        digits(state.cur_stage + 1, (20 - 7 - 3) * 8, 19 * 8);
        unsigned int percent =
                (unsigned int)(100.0 * state.cleared_stages / nstages);
        if (percent == 100) {
                *DRAW_COLORS = 0x03;
        } else {
                *DRAW_COLORS = 0x04;
        }
        digits(percent, (20 - 2 - 3) * 8, 19 * 8);

        if (meta.message == NULL) {
                return;
        }
        int start_y = meta.stage_height + 1;
        *DRAW_COLORS = 0x04;
        text(meta.message, 0, start_y * 8);
        int x = 0;
        int y = start_y;
        const uint8_t *p = (const uint8_t *)meta.message;
        uint8_t ch;
        while ((ch = *p++) != 0) {
                if (ch == '\n') {
                        x = 0;
                        y++;
                        continue;
                }
                if (0x90 <= ch && ch <= 0x99) {
                        uint8_t objidx = ch - 0x90;
                        draw_object(x, y, objidx);
                }
                x++;
        }
}

void
calc_stage_meta(map_t map, struct stage_meta *meta)
{
        int nplayers = 0;
        int nbombs = 0;
        int loc;
        for (loc = 0; loc < width * height; loc++) {
                uint8_t objidx = map[loc];
                if (is_player(objidx)) {
                        struct player *p = &meta->players[nplayers++];
                        p->loc = loc;
                } else if (is_bomb(objidx)) {
                        nbombs++;
                }
        }
        meta->nplayers = nplayers;
        meta->cur = 0;
        meta->nbombs = nbombs;
}

void
load_stage()
{
        struct map_info info;
        decode_stage(state.cur_stage, map, &info);
        /* move to the center of the screen */
        int d = (width - info.w) / 2;
        if (d > 0) {
                memmove(&map[d], map, (size_t)(width * height - d));
                memset(map, 0, (size_t)d);
        }
        calc_stage_meta(map, &meta);
        meta.stage_height = info.h;
        meta.message = info.message;
        mark_redraw_all();
        moving_step = 0;
        update_beam();
}

void
load_state()
{
        memset(&state, 0, sizeof(state));
        diskr(&state, sizeof(state));
        if (state.version != save_data_version) {
                memset(&state, 0, sizeof(state));
        }
}

void
save_state()
{
        state.version = save_data_version;
        diskw(&state, sizeof(state));
}

#define MOVE_OK 0x01
#define MOVE_PUSH 0x02     /* push something */
#define MOVE_BEAM 0x04     /* might need to recalculate beam */
#define MOVE_GET_BOMB 0x08 /* got a bomb */

unsigned int
player_move(struct stage_meta *meta, struct player *p, enum diridx dir,
            map_t map, map_t beam_map, bool commit)
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
                        if (commit) {
                                if (is_player(objidx)) {
                                        struct player *p2 =
                                                player_at(meta, loc);
                                        p2->loc = nloc;
                                }
                                move_object(nloc, loc);
                        }
                        if (block_beam(objidx)) {
                                flags |= MOVE_BEAM;
                        }
                }
        }
        if ((flags & MOVE_OK) != 0 && commit) {
                move_object(loc, p->loc);
                p->loc = loc;
        }
        return flags;
}

unsigned int
move(enum diridx dir)
{
        ASSERT(moving_dir != NONE);
        struct player *p = cur_player();
        return player_move(&meta, p, dir, map, beam[beamidx], true);
}

void
update_palette()
{
        unsigned int phase = frame / 8;
        unsigned int v = ((phase & 0x04) != 0 ? -phase - 1 : phase) & 0x03;
        PALETTE[2] = 0x111100 * (v + 2);
}

void
stage_clear()
{
        // trace("clear");
        uint32_t mask = 1 << (state.cur_stage % 32);
        if ((state.clear_bitmap[state.cur_stage / 32] & mask) == 0) {
                state.cleared_stages++;
        }
        state.clear_bitmap[state.cur_stage / 32] |= mask;
        state.cur_stage = (state.cur_stage + 1) % nstages;
        save_state();
        load_stage();
}

void
start()
{
        PALETTE[0] = 0x000030;
        PALETTE[1] = 0xc00000;
        PALETTE[2] = 0xffff00;
        PALETTE[3] = 0xa0a0a0;
        *SYSTEM_FLAGS = SYSTEM_PRESERVE_FRAMEBUFFER;

        load_state();
        load_stage();
}

void
update()
{
        uint8_t gamepad = *GAMEPAD1;
        uint8_t cur = gamepad;
        gamepad &= ~prev_gamepad;
        prev_gamepad = cur;
        if (moving_step == 0) {
                enum diridx dir = NONE;
                if ((gamepad & BUTTON_LEFT) != 0) {
                        dir = LEFT;
                } else if ((gamepad & BUTTON_RIGHT) != 0) {
                        dir = RIGHT;
                } else if ((gamepad & BUTTON_UP) != 0) {
                        dir = UP;
                } else if ((gamepad & BUTTON_DOWN) != 0) {
                        dir = DOWN;
                } else if ((gamepad & BUTTON_1) != 0) {
                        mark_redraw_cur_player();
                        switch_player();
                }

                if ((cur & BUTTON_2) != 0) {
                        if (dir == RIGHT || dir == LEFT) {
                                // trace("switch stage");
                                state.cur_stage =
                                        (state.cur_stage + nstages +
                                         (unsigned int)loc_x(
                                                 dirs[dir].loc_diff)) %
                                        nstages;
                                save_state();
                                load_stage();
                                return;
                        }
                        if (dir == UP) {
                                // trace("reset");
                                load_stage();
                                return;
                        }
                }

                if (dir != NONE) {
                        unsigned int flags = move(dir);
                        if ((flags & MOVE_OK) != 0) {
                                ASSERT(moving_step == 0);
                                moving_step++;
                                moving_dir = dir;
                                moving_beam = ((~flags &
                                                (MOVE_PUSH | MOVE_BEAM)) == 0);
                                moving_pushing = (flags & MOVE_PUSH) != 0;
                                if ((flags & MOVE_BEAM) != 0) {
                                        need_redraw |= CALC_BEAM;
                                }
                        }
                }
                mark_redraw_cur_player();
        }

        frame++;
        update_palette();
        if ((need_redraw & CALC_BEAM) != 0) {
                update_beam();
                need_redraw &= ~CALC_BEAM;
        }
        draw_beam();
        draw_objects();
        if ((need_redraw & MESSAGE) != 0) {
                draw_message();
        }

        if (moving_step) {
                moving_step++;
                if (moving_step == moving_nsteps) {
                        moving_step = 0;
                        mark_redraw_all_objects();
                }
        } else {
                need_redraw = 0;
                redraw_rect.xmin = width;
                redraw_rect.xmax = 0;
                redraw_rect.ymin = height;
                redraw_rect.ymax = 0;

                if (meta.nbombs == 0) {
                        stage_clear();
                }
        }
}
