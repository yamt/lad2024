#include <stdint.h>

#include "defs.h"

#define	C_R "\x93"

#define	C_B "\x96"
#define	C_X "\x97"
#define	C_P "\x98"
#define	C_A "\x99"

/* clang-format off */

const struct stage stages[] = {
    {
        .data = (const uint8_t[]){
            _, _, W, W, W, END,
            _, _, W, B, W, END,
            _, _, W, _, W, END,
            W, W, W, _, W, W, W, END,
            W, B, _, A, _, L, W, END,
            W, W, W, _, W, W, W, END,
            _, _, W, B, W, END,
            _, _, W, _, W, END,
            _, _, W, U, W, END,
            _, _, W, W, W, END,
            END,
        },
        .message =
        "Move "C_A" with \x84\x85\x86\x87 and\n"
        "collect all "C_B".\n"
    },

    {
        .data = (const uint8_t[]){
            _, _, W, W, W, END,
            _, W, _, _, A, W, END,
            W, R, B, _, B, _, W, END,
            W, _, B, _, B, _, W, END,
            W, _, B, _, B, L, W, END,
            _, W, _, _, U, W, END,
            _, _, W, W, W, END,
            END,
        },
        .message =
        C_A" can only move when\n"
        "the beam is hitting\n"
        "it.\n"
        C_R" emits the beam.\n"
        "\n"
        "\x81 to give up and\n"
        "reset the stage.",
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, W, END,
            W, _, _, _, A, _, W, END,
            W, _, R, _, _, L, W, END,
            W, _, _, _, _, _, W, END,
            W, _, B, _, _, _, W, END,
            W, _, _, _, _, A, W, END,
            W, _, _, _, _, U, W, END,
            _, W, W, W, W, W, END,
            END,
        },
        .message =
        "\x80 to switch between\n"
        C_A".\n"
        "\n"
        C_A" can push objects\n"
        "like "C_R" and other "C_A".",
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, R, _, _, W, END,
            W, _, A, P, _, W, END,
            W, _, _, B, _, W, END,
            _, W, W, _, _, W, END,
            _, _, _, W, W, END,
            END,
        },
        .message =
        "\x80 to switch between\n"
        C_A" and "C_P".\n"
        "\n"
        C_P" can not collect "C_B".\n"
        C_P" can push "C_A".\n"
        "\n"
        C_P" can move when the\n"
        "beam is NOT hitting\n"
        "it.\n"
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, _, _, _, W, END,
            W, B, _, B, _, W, END,
            W, _, B, _, _, W, END,
            W, B, P, B, W, W, END,
            W, _, A, L, _, W, END,
            W, _, _, _, _, W, END,
            _, W, W, W, W, END,
            END,
        },
        .message =
        "You can select a\n"
        "stage to play with\n"
        "\x81+\x85 and \x81+\x84.",
    },

    {
        .data = (const uint8_t[]){
            W, W, W, W, W, END,
            W, D, _, A, _, W, END,
            W, B, _, L, _, W, END,
            W, _, _, P, _, W, END,
            W, U, R, P, _, W, END,
            W, _, _, _, W, END,
            _, W, W, W, W, END,
            END,
        },
        .message =
        "The color of the\n"
        "stage number at\n"
        "the bottom of the\n"
        "screen shows if you\n"
        "have cleared the\n"
        "stage."
    },

    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, W, END,
            _, _, W, _, _, _, _, _, W, END,
            W, W, W, _, _, L, _, _, W, END,
            W, _, _, B, _, _, _, _, W, END,
            W, _, _, B, P, U, D, _, W, END,
            W, _, D, _, W, _, _, _, W, END,
            W, _, _, _, L, A, _, _, W, END,
            W, B, R, _, _, _, _, _, W, END,
            W, _, _, _, _, _, _, B, W, END,
            W, _, _, W, W, W, W, W, END,
            W, W, W, END,
            END,
        },
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, _, D, P, W, END,
            W, _, A, X, B, W, END,
            W, _, X, _, _, W, END,
            _, W, W, _, W, END,
            _, _, _, W, END,
            END,
        },
        .message =
        C_X" is same as "C_B"\n"
        "except that it\n"
        "blocks the beam.",
    },
};

/* clang-format on */

const unsigned int nstages = sizeof(stages) / sizeof(stages[0]);
