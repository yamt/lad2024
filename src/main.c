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

const uint8_t bomb[] = {
        0b00000000,
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b01011010,
        0b00111100,
        0b00000000,
};

const uint8_t bigbomb[] = {
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
                        .sprite = bomb,
                        .color = 0x20,
                        .flags = 0,
                },
        [X] =
                {
                        .sprite = bigbomb,
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

#define max_players 4

static uint8_t prev_gamepad = 0;
static unsigned int frame = 0;

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
struct save_data {
        uint32_t cur_stage;
        uint32_t clear_bitmap[(max_stages + 32 - 1) / 32];
} state;

#define width 20
#define height 20

uint8_t map[height][width];
uint8_t beam[height][width];

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
        return objidx == B || objidx == X;
}

bool
can_push(uint8_t objidx)
{
        return is_light(objidx) || is_player(objidx) || is_bomb(objidx);
}

int
light_dir(uint8_t objidx)
{
        return objidx - L;
}

const struct dir {
        int dx;
        int dy;
} dirs[] = {
        {
                -1,
                0,
        },
        {
                0,
                1,
        },
        {
                1,
                0,
        },
        {
                0,
                -1,
        },
};

bool
block_beam(uint8_t objidx)
{
        return is_light(objidx) || objidx == W || objidx == X;
}

bool
in_map(int x, int y)
{
        return 0 <= x && x < width && 0 <= y && y < height;
}

void
move_object(int nx, int ny, int ox, int oy)
{
        map[ny][nx] = map[oy][ox];
        map[oy][ox] = _;
}

void
calc_beam()
{
        int x;
        int y;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        beam[y][x] = 0;
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
                                beam[by][bx] = 1;
                        }
                }
        }
}

void
draw_beam()
{
        int x;
        int y;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        if (beam[y][x]) {
                                *DRAW_COLORS = 3;
                        } else {
                                *DRAW_COLORS = 1;
                        }
                        rect(x * 8, y * 8, 8, 8);
                }
        }
}

void
draw_object(int x, int y, uint8_t objidx)
{
        const struct obj *obj = &objs[objidx];
        *DRAW_COLORS = obj->color;
        int i = 0;
        if (is_player(objidx) && is_cur_player(x, y)) {
                i = (frame / 8) % 3;
        }
        blit(obj->sprite + i * 8, x * 8, y * 8, 8, 8, obj->flags);
}

void
draw_objects()
{
        int x;
        int y;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
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
        text("STAGE", (20 - 3 - 6) * 8, 19 * 8);
        if ((state.clear_bitmap[state.cur_stage / 32] &
             1 << (state.cur_stage % 32)) == 0) {
                *DRAW_COLORS = 0x02;
        }
        digits(state.cur_stage + 1, (20 - 3) * 8, 19 * 8);

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
        const uint8_t *p = stage->data;
        uint8_t ch;
        while ((ch = *p++) != END) {
                do {
                        map[y][x++] = ch;
                } while ((ch = *p++) != END);
                x = 0;
                y++;
        }
        calc_stage_meta();
        meta.stage_height = y;
        meta.message = stage->message;
}

void
load_state()
{
        memset(&state, 0, sizeof(state));
        diskr(&state, sizeof(state));
}

void
save_state()
{
        diskw(&state, sizeof(state));
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
        int dx = 0;
        int dy = 0;
        if ((gamepad & BUTTON_LEFT) != 0) {
                dx = -1;
        } else if ((gamepad & BUTTON_RIGHT) != 0) {
                dx = 1;
        } else if ((gamepad & BUTTON_UP) != 0) {
                dy = -1;
        } else if ((gamepad & BUTTON_DOWN) != 0) {
                dy = 1;
        } else if ((gamepad & BUTTON_1) != 0) {
                switch_player();
        } else if ((gamepad & BUTTON_2) != 0) {
                trace("reset");
                load_stage();
                return;
        }

        if (dx != 0 && (cur & BUTTON_2) != 0) {
                trace("switch stage");
                state.cur_stage =
                        (state.cur_stage + nstages + (unsigned int)dx) %
                        nstages;
                save_state();
                load_stage();
                return;
        }

        if (dx != 0 || dy != 0) {
                struct player *p = cur_player();
                bool is_robot = map[p->y][p->x] == A;
                if ((beam[p->y][p->x] != 0) == is_robot) {
                        int x = p->x + dx;
                        int y = p->y + dy;
                        if (in_map(x, y)) {
                                bool can_move = false;
                                uint8_t objidx = map[y][x];
                                if (is_robot && is_bomb(objidx)) {
                                        can_move = true;
                                        meta.nbombs--;
                                } else if (objidx == _) {
                                        can_move = true;
                                } else if (can_push(objidx)) {
                                        int nx = x + dx;
                                        int ny = y + dy;
                                        if (in_map(nx, ny) &&
                                            map[ny][nx] == _) {
                                                if (is_player(objidx)) {
                                                        struct player *p2 =
                                                                player_at(x,
                                                                          y);
                                                        p2->x = nx;
                                                        p2->y = ny;
                                                }
                                                move_object(nx, ny, x, y);
                                                can_move = true;
                                        }
                                }
                                if (can_move) {
                                        move_object(x, y, p->x, p->y);
                                        p->x = x;
                                        p->y = y;
                                }
                        }
                }
        }

        frame++;
        unsigned int phase = frame / 8;
        unsigned int v = ((phase & 0x04) != 0 ? -phase - 1 : phase) & 0x03;
        PALETTE[2] = 0x111100 * (v + 2);

        calc_beam();
        draw_beam();
        draw_objects();
        draw_message();

        if (meta.nbombs == 0) {
                trace("clear");
                state.clear_bitmap[state.cur_stage / 32] |=
                        1 << (state.cur_stage % 32);
                state.cur_stage = (state.cur_stage + 1) % nstages;
                save_state();
                load_stage();
        }
}
