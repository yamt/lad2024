#include <stdint.h>

#include "defs.h"

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
        "Move \x98 with \x84\x85\x86\x87 and\n"
        "collect all \x96.\n"
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
        "\x98 can only move when\n"
        "the beam is hitting\n"
        "it.\n"
        "\x93 emits the beam.\n"
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
        "\x98.\n"
        "\n"
        "\x98 can push objects\n"
        "like \x93 and other \x98.",
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
        "\x97 and \x98.\n"
        "\n"
        "\x97 can not collect \x96.\n"
        "\x97 can push \x96.\n"
        "\n"
        "\x97 can move when the\n"
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
};

/* clang-format on */

const unsigned int nstages = sizeof(stages) / sizeof(stages[0]);
