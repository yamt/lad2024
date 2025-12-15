#include "sprite.h"
#include "defs.h"
#include "wasm4.h"

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

const struct obj objs[] = {
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

const uint8_t *sprites[] = {
        sprites_8,
        scaled_sprites_16,
};

void
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
