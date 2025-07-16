#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "defs.h"
#include "hloader.h"
#include "rng.h"
#include "route.h"
#include "rule.h"
#include "util.h"
#include "wasm4.h"

#define VOLUME 32

enum sprite_idx {
        SPIDX_NONE = -1,
        SPIDX_PERSON = 0,
        SPIDX_PERSON1,
        SPIDX_PERSON2,
        SPIDX_ROBOT,
        SPIDX_ROBOT1,
        SPIDX_ROBOT2,
        SPIDX_BOX,
        SPIDX_BOMB,
        SPIDX_BOMB1,
        SPIDX_BOMB2,
        SPIDX_LIGHT,
        SPIDX_WALL,
        SPIDX_END
};

/* clang-format off */

static const uint8_t sprites_8[] = {
        /* PERSON */
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

        /* ROBOT */
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

        /* BOX */
        0b00000000,
        0b01110110,
        0b11110111,
        0b10001001,
        0b11110111,
        0b11110111,
        0b01110110,
        0b00000000,

        /* BOMB */
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b01111110,
        0b01111110,
        0b01011010,
        0b00111100,

        0b00000000,
        0b00111100,
        0b01011010,
        0b01111110,
        0b01111110,
        0b01111110,
        0b01011010,
        0b00111100,

        0b00000000,
        0b00000000,
        0b00111100,
        0b01011010,
        0b11111111,
        0b11111111,
        0b10111101,
        0b01111110,

        /* LIGHT */
        0b11111000,
        0b01111110,
        0b11111111,
        0b01111111,
        0b11111111,
        0b01111111,
        0b11111110,
        0b11111000,

        /* WALL */
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

static const struct obj {
        enum sprite_idx sprite;
        uint8_t color;
        uint8_t flags;
} objs[] = {
        [_] =
                {
                        .sprite = SPIDX_NONE,
                },
        [W] =
                {
                        .sprite = SPIDX_WALL,
                        .color = 0x40,
                        .flags = 0,
                },
        [U] =
                {
                        .sprite = SPIDX_LIGHT,
                        .color = 0x20,
                        .flags = BLIT_FLIP_X | BLIT_FLIP_Y | BLIT_ROTATE,
                },
        [R] =
                {
                        .sprite = SPIDX_LIGHT,
                        .color = 0x20,
                        .flags = BLIT_FLIP_X | BLIT_FLIP_Y,
                },
        [D] =
                {
                        .sprite = SPIDX_LIGHT,
                        .color = 0x20,
                        .flags = BLIT_ROTATE,
                },
        [L] =
                {
                        .sprite = SPIDX_LIGHT,
                        .color = 0x20,
                        .flags = 0,
                },
        [B] =
                {
                        .sprite = SPIDX_BOX,
                        .color = 0x20,
                        .flags = 0,
                },
        [X] =
                {
                        .sprite = SPIDX_BOMB,
                        .color = 0x20,
                        .flags = 0,
                },
        [P] =
                {
                        .sprite = SPIDX_PERSON,
                        .color = 0x40,
                        .flags = 0,
                },
        [A] =
                {
                        .sprite = SPIDX_ROBOT,
                        .color = 0x40,
                        .flags = 0,
                },
};

/* this one is mostly static */
static uint8_t scaled_sprites_16[2 * 16 * SPIDX_END];

static const uint8_t *sprites[] = {
        sprites_8,
        scaled_sprites_16,
};

/* global mutable states */

static unsigned int frame = 0;

static struct proj {
        unsigned int scale_idx;
        int x_offset;
        int y_offset;
} proj;

#define scale(x) ((x) << (proj.scale_idx + 3))
#define scale_x(x) (scale(x) + proj.x_offset)
#define scale_y(y) (scale(y) + proj.y_offset)
#define sprite(x) (&sprites[proj.scale_idx][(x) << (proj.scale_idx * 2 + 3)])

#define unscale(x) ((x) >> (proj.scale_idx + 3))
#define unscale_x(x) unscale((x)-proj.x_offset)
#define unscale_y(y) unscale((y)-proj.y_offset)

static unsigned int beamidx = 0;
static unsigned int cur_player_idx;

static enum animation_mode {
        NORMAL,
        CLEARED,
        GAVEUP,
} animation_mode;
static unsigned int animation_length = 0;
static unsigned int animation_frame = 0;

#define moving_nsteps 8
static unsigned int moving_speed;
static unsigned int moving_step = 0;
static bool undoing = false;
#define moving_dir undos[undo_idx].dir
#define moving_pushing ((undos[undo_idx].flags & MOVE_PUSH) != 0)
#define moving_beam ((~undos[undo_idx].flags & (MOVE_PUSH | MOVE_BEAM)) == 0)

struct move {
        loc_t loc;
        enum diridx dir;
        move_flags_t flags; /* MOVE_ flags */
};

#define max_undos 512
static struct move undos[max_undos];
static unsigned int undo_idx;

#define CALC_BEAM 1U
#define OUTSIDE 2U
#define MESSAGE 4U
#define ALL 7U
static unsigned int need_redraw;
static struct redraw_rect {
        int xmin;
        int ymin;
        int xmax;
        int ymax;
} redraw_rect;

static struct stage_meta meta;

static struct stage_draw_info {
        unsigned int message_y;
        const char *message;
} draw_info;

#define max_stages 800
#define save_data_version 1
struct save_data {
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t clear_bitmap[(max_stages + 32 - 1) / 32];
} state;

static map_t map;
static map_t beam[2];

static struct rng rng;

#define bomb_animate_nsteps 16
#define bomb_animate_nframes 5
static unsigned int bomb_animate_step;
static loc_t bomb_animate_loc;

static bool automove;
static map_t automove_route;

/* end of global mutable states */

bool
is_cur_player(loc_t loc)
{
        const struct player *p = &meta.players[cur_player_idx];
        return p->loc == loc;
}

struct player *
cur_player()
{
        return &meta.players[cur_player_idx];
}

void
switch_player()
{
        cur_player_idx = (cur_player_idx + 1) % meta.nplayers;
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

/*
 * mark a single object at the specified location to redraw
 */
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

/*
 * mark the current player to redraw
 */
void
mark_redraw_cur_player()
{
        struct player *p = cur_player();
        mark_redraw_object(p->loc);
}

/*
 * mark the all players to redraw
 */
static void
mark_redraw_for_stage_clear(void)
{
        unsigned int i;
        for (i = 0; i < meta.nplayers; i++) {
                struct player *p = &meta.players[i];
                mark_redraw_object(p->loc);
                if (p->loc >= map_width) {
                        mark_redraw_object(p->loc - map_width);
                        if (p->loc >= map_width * 2) {
                                mark_redraw_object(p->loc - map_width * 2);
                        } else {
                                /*
                                 * in this case, we need to redraw the
                                 * outside area. (draw_outside)
                                 */
                                need_redraw = OUTSIDE;
                        }
                }
        }
}

/*
 * mark all objects to redraw.
 * do NOT include ones overlapping with the status line and messages.
 */
void
mark_redraw_all_objects()
{
        /*
         * marking all objects simply by marking the first and last
         * locations at the stage.
         * XXX this assumes how mark_redraw_object is implemented
         *
         * `adj` below is to avoid overwriting the status line. ("STAGE 0000"
         * etc) (assuming only W, which is usually not necessary to redraw,
         * appears in the bottom line. actually, the assumption is a bit
         * wrong because there is a beam at the bottom-most row in stage 001.)
         */
        unsigned int adj = 1;
        if (state.cur_stage == 0) {
                /*
                 * stage 001 is exceptional as mentioned above.
                 * REVISIT: make this generic.
                 */
                adj = 0;
        }
        mark_redraw_object(genloc(0, 0));
        mark_redraw_object(genloc(map_width - 1, meta.stage_height - adj - 1));
}

void
clear_redraw_flags(void)
{
        need_redraw = 0;
        redraw_rect.xmin = map_width;
        redraw_rect.xmax = 0;
        redraw_rect.ymin = map_height;
        redraw_rect.ymax = 0;
}

/*
 * mark to redraw everything.
 * including the status line and messages.
 */
void
mark_redraw_all()
{
        clear_redraw_flags();
        need_redraw = ALL;
        mark_redraw_all_objects();
        redraw_rect.ymax = map_height;
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
                        prop = scale(moving_nsteps - moving_step) /
                               moving_nsteps;
                        loc_diff = -loc_diff;
                        curidx = 1 - curidx;
                } else {
                        prop = scale(moving_step) / moving_nsteps;
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
                                rect(scale_x(x), scale_y(y), scale(1),
                                     scale(1));
                        } else {
                                if (alt) {
                                        *DRAW_COLORS = 3;
                                } else {
                                        *DRAW_COLORS = 1;
                                }
                                if (horizontal) {
                                        rect(scale_x(x) + dx, scale_y(y),
                                             (uint32_t)(scale(1) - dx),
                                             scale(1));
                                } else {
                                        rect(scale_x(x), scale_y(y) + dy,
                                             scale(1),
                                             (uint32_t)(scale(1) - dy));
                                }
                                if (cur) {
                                        *DRAW_COLORS = 3;
                                } else {
                                        *DRAW_COLORS = 1;
                                }
                                if (horizontal) {
                                        rect(scale_x(x), scale_y(y),
                                             (uint32_t)dx, scale(1));
                                } else {
                                        rect(scale_x(x), scale_y(y), scale(1),
                                             (uint32_t)dy);
                                }
                        }
                }
        }
}

static void
set_proj(int x, int y, unsigned int n)
{
        ASSERT(n == 8 || n == 16);
        proj.x_offset = x;
        proj.y_offset = y;
        proj.scale_idx = n / 8 - 1;
}

static void
prepare_scaled_sprites(void)
{
        unsigned int i;
        for (i = 0; i < 8 * SPIDX_END; i++) {
                uint8_t orig = sprites_8[i];
                uint16_t scaled = 0;
                unsigned int j;
                for (j = 0; j < 8; j++) {
                        if ((orig & (1 << j)) != 0) {
                                scaled |= 3 << (j * 2);
                        }
                }
                scaled_sprites_16[i * 4] = scaled >> 8;
                scaled_sprites_16[i * 4 + 1] = (uint8_t)scaled;
                scaled_sprites_16[i * 4 + 2] = scaled >> 8;
                scaled_sprites_16[i * 4 + 3] = (uint8_t)scaled;
        }
}

static loc_t
mouse_loc(int x, int y)
{
        x = unscale_x(x);
        y = unscale_y(y);
        if (x < 0 || map_width <= x || y < 0 || map_height <= y) {
                return (loc_t)-1;
        }
        return genloc(x, y);
}

void
draw_object(int x, int y, uint8_t objidx)
{
        const struct obj *obj = &objs[objidx];
        *DRAW_COLORS = obj->color;
        loc_t loc = genloc(x, y);
        unsigned int i = 0;
        int dx = 0;
        int dy = 0;
        if (is_player(objidx)) {
                if (animation_mode == CLEARED) {
                        static const unsigned int ix[] = {0, 1, 2, 2,
                                                          2, 1, 0, 0};
                        static const int jump[] = {
                                4, 7, 9, 10, 10, 9, 7, 4,
                        };
                        i = (animation_frame / 4) %
                            (sizeof(ix) / sizeof(ix[0]));
                        i = ix[i];
                        *DRAW_COLORS = 0x30;
                        unsigned int r =
                                ((unsigned int)x * 101 + (unsigned int)y) %
                                16; /* kinda hash */
                        unsigned int jump_ix =
                                (animation_frame + r) %
                                (16 + sizeof(jump) / sizeof(jump[0]));
                        if (16 <= jump_ix) {
                                uint32_t jump_ix2 = jump_ix - 16;
                                dy = -(scale(jump[jump_ix2]) >> 3);
                                if (jump_ix2 == 0) {
                                        tone(110 | (340 << 16), 10 | (15 << 8),
                                             VOLUME, TONE_PULSE1);
                                }
                        }
                } else if (animation_mode == GAVEUP || is_cur_player(loc)) {
                        i = (frame / 8) % 3;
                }
        }
        if (is_bomb(objidx) && bomb_animate_step && bomb_animate_loc == loc) {
                i = bomb_animate_step * bomb_animate_nframes /
                    bomb_animate_nsteps;
                i = (const unsigned int[]){1, 2, 0, 1, 2}[i];
                // tracef("loc %d step %d frame %d", (int)loc,
                // bomb_animate_step, i);
        }
        if ((is_player(objidx) || can_push(objidx)) && is_moving(loc)) {
                const struct dir *dir = &dirs[moving_dir];
                int prop = scale(moving_nsteps - moving_step) / moving_nsteps;
                int loc_diff = -dir->loc_diff * prop;
                dx = loc_x(loc_diff);
                dy = loc_y(loc_diff);
        }
        if (animation_mode == GAVEUP && is_player(objidx)) {
                unsigned int scl = 1 << proj.scale_idx;
                int d = animation_frame / 6;
                int l;
                for (l = 0; l < 8; l++) {
                        int lidx = 8 - (8 - l) * 8 / (8 - d);
                        if (lidx < 0) {
                                continue;
                        }
                        blit(sprite((unsigned int)obj->sprite + i) +
                                     (unsigned int)lidx * scl * scl,
                             scale_x(x) + dx, scale_y(y) + dy + l * (int)scl,
                             scale(1), scl, obj->flags);
                }
        } else {
                blit(sprite((unsigned int)obj->sprite + i), scale_x(x) + dx,
                     scale_y(y) + dy, scale(1), scale(1), obj->flags);
        }
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

static void
bordered_text(const char *msg, int x, int y)
{
        uint16_t saved_colors = *DRAW_COLORS;
        *DRAW_COLORS = 0x01;
        text(msg, x + 1, y);
        text(msg, x, y - 1);
        text(msg, x - 1, y);
        text(msg, x, y + 1);
        *DRAW_COLORS = saved_colors;
        text(msg, x, y);
}

void
digits(unsigned int v, int x, int y)
{
        char buf[4];
        buf[0] = '0' + ((v / 100) % 10);
        buf[1] = '0' + ((v / 10) % 10);
        buf[2] = '0' + (v % 10);
        buf[3] = 0;
        bordered_text(buf, x, y);
}

unsigned int
count_message_lines(const char *message)
{
        const uint8_t *p = (const void *)message;
        unsigned int x = 0;
        unsigned int y = 0;
        uint8_t ch;
        while ((ch = *p++) != 0) {
                if (ch == '\n') {
                        x = 0;
                        y++;
                        continue;
                }
                x++;
        }
        if (x > 0) {
                y++;
        }
        return y;
}

void
draw_outside(void)
{
        if (proj.x_offset > 0) {
                *DRAW_COLORS = 1;
                rect(0, 0, (unsigned int)proj.x_offset, map_height * 20);
        }
        if (proj.y_offset > 0) {
                *DRAW_COLORS = 1;
                rect(0, 0, map_width * 8, (unsigned int)proj.y_offset);
        }
}

static unsigned int
cleared_percentage(uint32_t cleared_stages)
{
        return (unsigned int)(100.0 * cleared_stages / nstages);
}

void
draw_message()
{
        *DRAW_COLORS = 0x04;
        bordered_text("STAGE     (   %)", (20 - 7 - 3 - 6) * 8, 19 * 8);
        if ((state.clear_bitmap[state.cur_stage / 32] &
             1 << (state.cur_stage % 32)) == 0) {
                *DRAW_COLORS = 0x02;
        }
        digits(state.cur_stage + 1, (20 - 7 - 3) * 8, 19 * 8);
        unsigned int percent = cleared_percentage(state.cleared_stages);
        if (percent == 100 && animation_mode != CLEARED) {
                *DRAW_COLORS = 0x03;
        } else {
                *DRAW_COLORS = 0x04;
        }
        digits(percent, (20 - 2 - 3) * 8, 19 * 8);

        if (draw_info.message == NULL) {
                return;
        }
        // tracef("message_y %d", draw_info.message_y);
        int start_y = (int)draw_info.message_y;

        /*
         * note: we don't want to draw player characters within the messages
         * scaled or with the alternative color. reset proj and animation_mode
         * temporarily.
         */
        struct proj saved_proj = proj;
        set_proj(0, 0, 8);
        enum animation_mode saved_mode = animation_mode;
        animation_mode = NORMAL;

        *DRAW_COLORS = 0x04;
        text(draw_info.message, 0, start_y * 8);
        int x = 0;
        int y = start_y;
        const uint8_t *p = (const uint8_t *)draw_info.message;
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
        animation_mode = saved_mode;
        proj = saved_proj;
}

void
load_stage()
{
        struct map_info info;
        decode_huff_stage(state.cur_stage, map, &info);

        unsigned int lines = 0;
        if (info.message != NULL) {
                lines = count_message_lines(info.message);
                lines += 2;
        }
        ASSERT(info.w <= map_width);
        ASSERT(info.h + lines <= map_height);
        // tracef("w %d h %d lines %d", info.w, info.h, lines);
        unsigned int unit = 8;
        int xoff = 0;
        int yoff = 0;
        if (info.w * 16 / 8 <= map_width + 2 &&
            ((lines != 0 && info.h * 16 / 8 + lines <= map_height) ||
             (lines == 0 && info.h * 16 / 8 + lines <= map_height + 2))) {
                if (info.w * 16 / 8 > map_width) {
                        xoff = -8;
                } else if (((map_width * 8 / 16 - info.w) % 2) == 1) {
                        xoff = 8;
                }
                if (info.h * 16 / 8 + lines > map_height) {
                        /*
                         * note: -9 to avoid interfering the
                         * "STAGE ..." line.
                         */
                        yoff = -9;
                } else if ((((map_height - lines) * 8 / 16 - info.h) % 2) ==
                           1) {
                        yoff = 8;
                }
                unit = 16;
        }
        set_proj(xoff, yoff, unit);

        /*
         * move to the center of the screen
         *
         * we do this way instead of using larger x/y offset to set_proj
         * because this logic predates set_proj offset. probably we can
         * simplify this a bit.
         */
        unsigned int disp_width =
                (unsigned int)(map_width * 8 - xoff * 2) / unit;
        unsigned int disp_height =
                (unsigned int)((int)(map_height - lines) * 8 - yoff * 2) /
                unit;
        int dx = (int)(disp_width - info.w) / 2;
        int dy = (int)(disp_height - info.h) / 2;
        ASSERT(dx < map_width);
        ASSERT(dy < map_height);
        if (dx > 0 || dy > 0) {
                loc_t loc = genloc(dx, dy);
                memmove(&map[loc], map,
                        (size_t)(map_width * map_height - loc));
                memset(map, 0, (size_t)loc);
        }

        /*
         * reset misc global mutables states
         * XXX maybe it's simpler to group them and reset with a memset.
         *
         * frame: no need to reset
         * need_redraw/redraw_rect: will be set by mark_redraw_all
         * draw_info: will be set below
         * state: should not be reset
         * map: already set
         * beam_map: will be set by update_beam
         * rng: no need to reset
         * bomb_animate_loc: no need to reset
         */
        cur_player_idx = 0;
        moving_step = 0;
        memset(undos, 0, sizeof(undos));
        undo_idx = 0;
        undoing = false;
        bomb_animate_step = 0;

        calc_stage_meta(map, &meta);
        meta.stage_height = info.h + (unsigned int)dy;
        draw_info.message_y = map_height - lines;
        draw_info.message = info.message;
#if 0
        tracef("stage %d h %d lines %d stage_height %d unit %d yoff %d "
               "message_y %d",
               state.cur_stage + 1, info.h, lines, meta.stage_height, unit,
               yoff, draw_info.message_y);
#endif
        ASSERT(draw_info.message == NULL ||
               (int)(meta.stage_height * unit) + yoff <=
                       (int)draw_info.message_y * 8);

        mark_redraw_all();
        update_beam();
}

static void
validate_state(void)
{
        ASSERT(state.cur_stage < nstages);
        ASSERT(state.cleared_stages < nstages);
        unsigned int n = 0;
        unsigned int i;
        for (i = 0; i < max_stages; i++) {
                if ((state.clear_bitmap[i / 32] & (1 << (i % 32))) != 0) {
                        n++;
                }
        }
        ASSERT(state.cleared_stages == n);
}

void
load_state()
{
        memset(&state, 0, sizeof(state));
        diskr(&state, sizeof(state));
        if (state.version == 0) {
                /*
                 * initial state. (all zero)
                 *
                 * or maybe a save data from a version older than
                 * the first public release. (20240831)
                 */
                unsigned int i;
                for (i = 0; i < sizeof(state); i++) {
                        ASSERT(((const uint8_t *)&state)[i] == 0);
                }
                return;
        }
        ASSERT(state.version > 0 && state.version <= save_data_version);
        validate_state();
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

move_flags_t
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

static bool
bumping_cleared_percentage(void)
{
        uint32_t mask = 1 << (state.cur_stage % 32);
        if ((state.clear_bitmap[state.cur_stage / 32] & mask) != 0) {
                return false;
        }
        return cleared_percentage(state.cleared_stages + 1) !=
               cleared_percentage(state.cleared_stages);
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
undo_move(const struct move *undo)
{
        struct player *p = cur_player();
        ASSERT(p->loc == undo->loc);
        move_flags_t flags = undo->flags;
        loc_t diff = dirs[undo->dir].loc_diff;
        loc_t ploc = p->loc;
        ASSERT(map[ploc - diff] == _);
        move_object(map, ploc - diff, ploc);
        if ((flags & MOVE_PUSH) != 0) {
                move_object(map, ploc, ploc + diff);
                struct player *p2 = player_at(&meta, ploc + diff);
                if (p2 != NULL) {
                        p2->loc -= diff;
                }
        }
        if ((flags & MOVE_GET_BOMB) != 0) {
                map[ploc] = X;
                meta.nbombs++;
        }
        p->loc -= diff;
        ASSERT(is_player(map[p->loc]));
}

void
mark_redraw_after_move(const struct move *undo)
{
        loc_t diff = dirs[undo->dir].loc_diff;
        mark_redraw_object(undo->loc - diff);
        if ((undo->flags & MOVE_PUSH) != 0) {
                mark_redraw_object(undo->loc + diff);
        }
        if ((undo->flags & MOVE_BEAM) != 0) {
                need_redraw |= CALC_BEAM;
        }
}

void
animate_bomb(void)
{
        if (bomb_animate_step) {
                bomb_animate_step++;
                if (bomb_animate_step == bomb_animate_nsteps) {
                        bomb_animate_step = 0;
                }
                mark_redraw_object(bomb_animate_loc);
        } else {
                loc_t loc = rng_rand(&rng, 0, map_width * map_height - 1);
                if (map[loc] == X) {
                        // tracef("loc %d", (int)loc);
                        bomb_animate_loc = loc;
                        bomb_animate_step++;
                        mark_redraw_object(bomb_animate_loc);
                }
        }
}

void
read_gamepad(uint8_t *curp, uint8_t *pushedp, uint8_t *repeatedp)
{
        static uint8_t prev_gamepad = 0;
        static uint8_t holding_frames;

        uint8_t cur = *GAMEPAD1;
        uint8_t gamepad = cur;
        uint8_t gamepad_with_repeat;
        gamepad &= ~prev_gamepad;
        gamepad_with_repeat = gamepad;
        if (cur == prev_gamepad) {
                if (holding_frames < 40) {
                        holding_frames++;
                } else {
                        gamepad_with_repeat |= cur;
                }
        } else {
                holding_frames = 0;
        }
        prev_gamepad = cur;

        *curp = cur;
        *pushedp = gamepad;
        *repeatedp = gamepad_with_repeat;
}

void
read_mouse_buttons(uint8_t *curp, uint8_t *pushedp)
{
        static uint8_t prev = 0;

        uint8_t cur = *MOUSE_BUTTONS;
        uint8_t pushed = cur;
        pushed &= ~prev;
        prev = cur;

        *curp = cur;
        *pushedp = pushed;
}

enum diridx
gamepad_to_dir(uint8_t pad)
{
        enum diridx dir = NONE;
        if ((pad & BUTTON_LEFT) != 0) {
                dir = LEFT;
        } else if ((pad & BUTTON_RIGHT) != 0) {
                dir = RIGHT;
        } else if ((pad & BUTTON_UP) != 0) {
                dir = UP;
        } else if ((pad & BUTTON_DOWN) != 0) {
                dir = DOWN;
        }
        return dir;
}

static void
reset_palette(void)
{
        PALETTE[0] = 0x000030; /* background */
        PALETTE[1] = 0xc00000; /* movable objects */
        PALETTE[2] = 0xffff00; /* beams */
        PALETTE[3] = 0xa0a0a0; /* wall, players, text */
}

static void
reset_alt_palette(void)
{
        /*
         * an alternative palette used during the stage-clear animation.
         *
         * 0,1,3 are the darken version of the normal palette.
         * 2 is used to show jumping players.
         * note: no beams are shown during the animation.
         */

        PALETTE[0] = 0x000018;
        PALETTE[1] = 0x600000;
        PALETTE[2] = 0xa0a0a0; /* jumping players */
        PALETTE[3] = 0x505050;
}

static void
reset_alt_palette2(void)
{
        /*
         * an alternative palette used during the give-up animation.
         *
         * 1,2,3 are same as the normal palette.
         */

        PALETTE[0] = 0x300000; /* background (red) */
        PALETTE[1] = 0xc00000;
        PALETTE[2] = 0xffff00;
        PALETTE[3] = 0xa0a0a0;
}

void
start()
{
        ASSERT(nstages <= max_stages);

        reset_palette();
        *SYSTEM_FLAGS = SYSTEM_PRESERVE_FRAMEBUFFER;

        load_state();
        load_stage();

        rng_init(&rng, 0);

        prepare_scaled_sprites();
}

void
update()
{
        uint8_t gamepad_cur;
        uint8_t gamepad;
        uint8_t gamepad_with_repeat;
        read_gamepad(&gamepad_cur, &gamepad, &gamepad_with_repeat);
        if (animation_mode == CLEARED) {
                mark_redraw_for_stage_clear();
        } else if (animation_mode == GAVEUP) {
                mark_redraw_for_stage_clear(); /* XXX abuse */
        } else if (moving_step == 0) {
                uint8_t mouse_buttons_cur;
                uint8_t mouse_buttons;
                read_mouse_buttons(&mouse_buttons_cur, &mouse_buttons);

                enum diridx dir = NONE;
                if (automove) {
                        gamepad_cur = 0;
                } else if ((mouse_buttons & MOUSE_LEFT) != 0) {
                        const loc_t loc = mouse_loc(*MOUSE_X, *MOUSE_Y);
                        if (loc != (loc_t)-1) {
                                const struct player *p = cur_player();

                                gamepad_cur = 0;
                                enum diridx i;
                                for (i = 0; i < 4; i++) {
                                        if (p->loc + dir_loc_diff(i) == loc) {
                                                dir = i;
                                                break;
                                        }
                                }
                                if (dir == NONE) {
                                        route_calculate(map, beam[beamidx],
                                                        loc, p->loc,
                                                        map[p->loc] == A,
                                                        automove_route);
                                        automove = true;
                                }
                        }
                } else {
                        dir = gamepad_to_dir(gamepad);
                }

                if (automove) {
                        const struct player *p = cur_player();
                        dir = (enum diridx)automove_route[p->loc];
                        if (dir == NONE) {
                                automove = false;
                        }
                }

                if (dir == NONE && (gamepad & BUTTON_1) != 0) {
                        mark_redraw_cur_player();
                        switch_player();
                }

                if ((gamepad_cur & BUTTON_2) != 0) {
                        enum diridx rdir = gamepad_to_dir(gamepad_with_repeat);
                        if (rdir == RIGHT || rdir == LEFT) {
                                // trace("switch stage");
                                state.cur_stage =
                                        (state.cur_stage + nstages +
                                         (unsigned int)loc_x(
                                                 dirs[rdir].loc_diff)) %
                                        nstages;
                                save_state();
                                load_stage();
                        } else if (dir == UP) {
                                // trace("reset");
                                tone(440 | (10 << 16), 40 | (8 << 8), 80,
                                     TONE_TRIANGLE);
                                animation_mode = GAVEUP;
                                animation_frame = 0;
                                return;
                        } else if (dir == DOWN) {
                                const struct move *undo = &undos[undo_idx];
                                if ((undo->flags & MOVE_OK) != 0) {
                                        // trace("undo");
                                        move_flags_t flags = undo->flags;

                                        /*
                                         * switch back to the last-moved
                                         * player
                                         */
                                        const struct player *p =
                                                player_at(&meta, undo->loc);
                                        ASSERT(p != NULL);
                                        if (cur_player() != p) {
                                                mark_redraw_cur_player();
                                                cur_player_idx =
                                                        (unsigned int)(p -
                                                                       meta.players);
                                                ASSERT(cur_player() == p);
                                        }

                                        /* undo, recalculate beam, redo */
                                        if ((flags & MOVE_BEAM) != 0) {
                                                undo_move(undo);
                                                update_beam();
                                                flags = move(undo->dir);
                                                ASSERT(flags == undo->flags);
                                                need_redraw |= CALC_BEAM;
                                        }
                                        mark_redraw_after_move(undo);
                                        mark_redraw_cur_player();

                                        undoing = true;
                                        moving_speed = 2;
                                        moving_step =
                                                moving_nsteps - moving_speed;
                                }
                        }
                } else if (dir != NONE) {
                        move_flags_t flags = move(dir);
                        if ((flags & MOVE_OK) != 0) {
                                ASSERT(moving_step == 0);

                                undo_idx = (undo_idx + 1) % max_undos;
                                struct move *undo = &undos[undo_idx];
                                const struct player *p = cur_player();
                                undo->loc = p->loc;
                                undo->dir = dir;
                                undo->flags = flags;
                                mark_redraw_after_move(undo);

                                moving_speed = 2;
                                if (map[cur_player()->loc] == A) {
                                        static unsigned int toggle;
                                        toggle = 1 - toggle;
                                        tone((110 + 100 * toggle) |
                                                     (160 << 16),
                                             4, VOLUME, TONE_PULSE1);
                                } else if ((flags & MOVE_PUSH) != 0) {
                                        /*
                                         * when P is pushing something,
                                         * make it slow.
                                         */
                                        moving_speed = 1;
                                        tone(270, 4 << 24,
                                             (VOLUME << 8) | VOLUME,
                                             TONE_NOISE);
                                }
                                if ((flags & MOVE_GET_BOMB)) {
                                        tone(400, (2 << 16) | 8 | (30 << 8),
                                             (VOLUME << 8) | (VOLUME * 6 / 16),
                                             TONE_NOISE);
                                }
                                moving_step += moving_speed;
                        }
                }
                mark_redraw_cur_player();
        }

        frame++;
        if (animation_mode == CLEARED) {
                reset_alt_palette();
        } else {
                if (animation_mode == GAVEUP) {
                        reset_alt_palette2();
                } else {
                        reset_palette();
                }
                update_palette();
        }
        animate_bomb();
        if ((need_redraw & CALC_BEAM) != 0) {
                update_beam();
                need_redraw &= ~CALC_BEAM;
        }
        draw_beam();
        if ((need_redraw & OUTSIDE) != 0) {
                /*
                 * this should be before draw_objects because
                 * player characters, which are drawn by draw_objects,
                 * can jump into the "outside" area during the stage
                 * clear animation.
                 */
                draw_outside();
        }
        draw_objects();
        if ((need_redraw & MESSAGE) != 0) {
                /*
                 * this should be after draw_object because the status
                 * text overlays on objects for some stages.
                 */
                draw_message();
        }

        if (animation_mode == CLEARED) {
                clear_redraw_flags();
                animation_frame++;
                if (animation_frame == (2 * 8 + 5) * animation_length) {
                        animation_mode = NORMAL;
                        stage_clear();
                }
        } else if (animation_mode == GAVEUP) {
                clear_redraw_flags();
                animation_frame++;
                if (animation_frame == 6 * 8) {
                        animation_mode = NORMAL;
                        load_stage();
                }
        } else if (moving_step) {
                if (undoing) {
                        // tracef("undo step %d", moving_step);
                        moving_step -= moving_speed;
                        if (moving_step == 0) {
                                struct move *undo = &undos[undo_idx];
                                undo_move(undo);
                                mark_redraw_after_move(undo);
                                undoing = false;
                                undo->flags = 0; /* mark this invalid */
                                undo_idx =
                                        (undo_idx + max_undos - 1) % max_undos;
                        }
                } else {
                        moving_step += moving_speed;
                        if (moving_step == moving_nsteps) {
                                moving_step = 0;
                                mark_redraw_all_objects();
                        }
                }
        } else {
                clear_redraw_flags();

                if (meta.nbombs == 0) {
                        mark_redraw_all_objects();
                        /*
                         * note: as wasm-4 has only 4 colors, we turn off
                         * all lights here so that we can use the color
                         * for beams for the other purpose. (ie. animate
                         * players. see reset_alt_palette)
                         */
                        memset(beam[beamidx], 0, map_width * map_height);
                        /*
                         * note: "100%" is also drawn with the same color as
                         * beams. redraw it with the normal color.
                         */
                        need_redraw = MESSAGE;
                        animation_mode = CLEARED;
                        if (bumping_cleared_percentage()) {
                                animation_length = 16;
                        } else {
                                animation_length = 4;
                        }
                        animation_frame = 1;
                }
        }
}
