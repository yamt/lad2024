#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "wasm4.h"

#include "defs.h"

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

enum diridx {
        NONE = -1,
        LEFT = 0,
        DOWN = 1,
        RIGHT = 2,
        UP = 3,
};

const struct dir {
        int dx;
        int dy;
} dirs[] = {
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
        int nbombs;

        int stage_height;
        const char *message;
} meta;

struct player {
        int x;
        int y;
} players[max_players];

#define max_stages 100
#define save_data_version 1
struct save_data {
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t clear_bitmap[(max_stages + 32 - 1) / 32];
} state;

#define width 20
#define height 20

typedef uint8_t map_t[height][width];
map_t map;
map_t beam[2];

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
is_cur_player(int x, int y)
{
        const struct player *p = &players[meta.cur];
        return p->x == x && p->y == y;
}

struct player *
cur_player()
{
        return &players[meta.cur];
}

struct player *
player_at(int x, int y)
{
        int i;
        for (i = 0; i < meta.nplayers; i++) {
                struct player *p = &players[i];
                if (p->x == x && p->y == y) {
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

bool
is_moving(int x, int y)
{
        if (moving_step == 0) {
                return false;
        }
        if (is_cur_player(x, y)) {
                return true;
        }
        if (!moving_pushing) {
                return false;
        }
        const struct dir *d = &dirs[moving_dir];
        return is_cur_player(x - d->dx, y - d->dy);
}

void
mark_redraw_object(int x, int y)
{
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
        mark_redraw_object(p->x, p->y);
}

void
mark_redraw_all_objects()
{
        /* XXX this assumes how mark_redraw_object is implemented */
        mark_redraw_object(0, 0);
        mark_redraw_object(width - 1, meta.stage_height - 1);
}

void
mark_redraw_all()
{
        need_redraw = ALL;
        mark_redraw_all_objects();
        redraw_rect.ymax = height;
}

void
move_object(int nx, int ny, int ox, int oy)
{
        mark_redraw_object(ox, oy);
        mark_redraw_object(nx, ny);
        map[ny][nx] = map[oy][ox];
        map[oy][ox] = _;
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
                dx = d->dx;
                dy = d->dy;
                horizontal = dx != 0;
                if (dx < 0 || dy < 0) {
                        prop = 8 * (moving_nsteps - moving_step) /
                               moving_nsteps;
                        dx = -dx;
                        dy = -dy;
                        curidx = 1 - curidx;
                } else {
                        prop = 8 * moving_step / moving_nsteps;
                        altidx = 1 - curidx;
                }
                dx = dx * prop;
                dy = dy * prop;
        }
        int x;
        int y;
        for (y = redraw_rect.ymin; y < redraw_rect.ymax; y++) {
                for (x = redraw_rect.xmin; x < redraw_rect.xmax; x++) {
                        uint8_t cur = beam[curidx][y][x];
                        uint8_t alt = beam[altidx][y][x];
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
        int i = 0;
        int dx = 0;
        int dy = 0;
        if (is_player(objidx) && is_cur_player(x, y)) {
                i = (frame / 8) % 3;
        }
        if ((is_player(objidx) || can_push(objidx)) && is_moving(x, y)) {
                const struct dir *dir = &dirs[moving_dir];
                int prop = 8 * (moving_nsteps - moving_step) / moving_nsteps;
                dx = -dir->dx * prop;
                dy = -dir->dy * prop;
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
                        uint8_t objidx = map[y][x];
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
calc_stage_meta()
{
        int x;
        int y;
        int nplayers = 0;
        int nbombs = 0;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        uint8_t objidx = map[y][x];
                        if (is_player(objidx)) {
                                struct player *p = &players[nplayers++];
                                p->x = x;
                                p->y = y;
                        } else if (is_bomb(objidx)) {
                                nbombs++;
                        }
                }
        }
        meta.nplayers = nplayers;
        meta.cur = 0;
        meta.nbombs = nbombs;
}

void
load_stage()
{
        const struct stage *stage = &stages[state.cur_stage];

        memset(map, 0, sizeof(map));
        int x;
        int y;
        x = y = 0;
        int xmax = 0;
        const uint8_t *p = stage->data;
        uint8_t ch;
        while ((ch = *p++) != END) {
                do {
                        map[y][x++] = ch;
                        if (xmax < x) {
                                xmax = x;
                        }
                } while ((ch = *p++) != END);
                x = 0;
                y++;
        }
        /* move to the center of the screen */
        int d = (width - xmax) / 2;
        if (d > 0) {
                memmove(&map[0][d], map, (size_t)(width * height - d));
                memset(map, 0, (size_t)d);
        }
        calc_stage_meta();
        meta.stage_height = y;
        meta.message = stage->message;
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

void
move(enum diridx dir)
{
        ASSERT(moving_dir != NONE);
        struct player *p = cur_player();
        bool is_robot = map[p->y][p->x] == A;
        const struct dir *d = &dirs[dir];
        int dx = d->dx;
        int dy = d->dy;
        if ((beam[beamidx][p->y][p->x] != 0) == is_robot) {
                int x = p->x + dx;
                int y = p->y + dy;
                if (in_map(x, y)) {
                        bool can_move = false;
                        uint8_t objidx = map[y][x];
                        moving_pushing = false;
                        moving_beam = false;
                        if (is_robot && is_bomb(objidx)) {
                                can_move = true;
                                meta.nbombs--;
                                need_redraw |= CALC_BEAM;
                        } else if (objidx == _) {
                                can_move = true;
                        } else if (can_push(objidx)) {
                                int nx = x + dx;
                                int ny = y + dy;
                                if (in_map(nx, ny) && map[ny][nx] == _) {
                                        if (is_player(objidx)) {
                                                struct player *p2 =
                                                        player_at(x, y);
                                                p2->x = nx;
                                                p2->y = ny;
                                        }
                                        move_object(nx, ny, x, y);
                                        can_move = true;
                                        moving_pushing = true;
                                        if (block_beam(objidx)) {
                                                need_redraw |= CALC_BEAM;
                                                moving_beam = true;
                                        }
                                }
                        }
                        if (can_move) {
                                move_object(x, y, p->x, p->y);
                                p->x = x;
                                p->y = y;
                                ASSERT(moving_step == 0);
                                moving_step++;
                                moving_dir = dir;
                        }
                }
        }
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
                                         (unsigned int)dirs[dir].dx) %
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
                        move(dir);
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
