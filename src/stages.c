#include <stdint.h>

#include "defs.h"
#include "stages.h"

#define C_B "\x92"
#define C_X "\x93"

#define C_R "\x96"

#define C_P "\x98"
#define C_A "\x99"

/* clang-format off */

const struct stage stages[] = {
    {
        .data = (const uint8_t[]){
            W, W, W, END,
            W, D, W, END,
            W, A, W, END,
            W, _, W, _, _, _, _, R, _, _, D, _, _, W, W, W, W, _, END,
            W, _, W, _, _, _, _, _, _, _, _, _, _, W, _, _, _, W, END,
            W, _, W, W, W, W, _, D, _, L, P, _, _, W, _, P, _, W, END,
            W, X, X, _, L, W, _, _, _, _, _, _, _, W, _, _, _, W, END,
            W, W, W, W, W, W, _, R, _, _, U, L, _, W, W, W, W, END,
            END,
        },
        .message =
        "Move "C_A" with \x84\x85\x86\x87 and\n"
        "collect all "C_X".\n"
    },

    {
        .data = (const uint8_t[]){
            _, _, W, W, W, END,
            _, W, _, _, X, W, END,
            W, R, _, X, _, X, W, END,
            W, _, X, _, A, _, W, END,
            W, X, _, X, _, L, W, END,
            _, W, _, _, U, W, END,
            _, _, W, W, W, END,
            END,
        },
        .message =
        C_R" emits the beam.\n"
        "\n"
        C_A" can only move when\n"
        "the beam is hitting\n"
        "it.\n"
        "\n"
        "\x81+\x86 to give up and\n"
        "reset the stage.",
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, W, END,
            W, _, _, _, A, _, W, END,
            W, _, R, _, _, L, W, END,
            W, _, _, _, _, _, W, END,
            W, _, X, _, _, _, W, END,
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
            W, _, _, X, _, W, END,
            _, W, W, _, _, W, END,
            _, _, _, W, W, END,
            END,
        },
        .message =
        "\x80 to switch between\n"
        C_A" and "C_P".\n"
        "\n"
        C_P" can not collect "C_X".\n"
        C_P" can push objects\n"
        "including "C_A" and "C_X".\n"
        "\n"
        C_P" can move only\n"
        "when the beam is NOT\n"
        "hitting it.\n"
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, _, _, _, W, END,
            W, X, _, X, _, W, END,
            W, _, X, _, _, W, END,
            W, X, P, X, W, W, END,
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
            W, X, _, L, _, W, END,
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
            W, _, _, X, _, _, _, _, W, END,
            W, _, _, X, P, U, D, _, W, END,
            W, _, D, _, W, _, _, _, W, END,
            W, _, _, _, L, A, _, _, W, END,
            W, X, R, _, _, _, _, _, W, END,
            W, _, _, _, _, _, _, X, W, END,
            W, _, _, W, W, W, W, W, END,
            W, W, W, END,
            END,
        },
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, _, _, W, W, END,
            W, _, D, _, P, W, END,
            W, _, _, X, _, W, END,
            W, W, B, _, _, W, END,
            W, W, _, X, _, W, END,
            W, W, B, _, _, W, END,
            _, W, _, _, W, END,
            _, W, _, _, W, END,
            _, W, B, _, W, END,
            _, W, _, _, W, END,
            _, W, _, _, W, END,
            _, W, A, X, W, END,
            _, _, W, W, _, END,
            END,
        },
        .message =
        C_B" blocks the beam.\n"
        C_A" and "C_P" can push "C_B".",
    },

    {
        .data = (const uint8_t[]){
            _, W, W, W, W, END,
            W, _, _, D, P, W, END,
            W, _, A, X, X, W, END,
            W, _, X, _, _, W, END,
            _, W, W, _, W, END,
            _, _, _, W, END,
            END,
        },
    },

    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, END,
            _, W, _, _, _, _, W, END,
            _, W, _, W, W, _, W, W, END,
            W, X, _, A, _, X, _, _, W, END,
            W, _, X, P, X, _, _, R, L, W, END,
            _, W, X, _, W, W, _, _, W, END,
            _, W, _, _, _, _, _, W, END,
            _, _, W, W, W, W, W, END,
            END,
        },
    },

#include "1994_stages.inc"

    {
        .data = (const uint8_t[]){
            _, _, _, _, _, _, _, _, W, W, W, END,
            _, _, _, _, _, _, _, W, _, _, W, END,
            _, _, _, _, _, _, W, _, _, L, W, END,
            _, _, _, _, _, W, _, _, P, _, W, END,
            _, _, _, _, W, _, _, B, _, W, END,
            _, _, _, W, _, _, B, _, W, END,
            _, _, W, _, _, B, _, W, END,
            _, W, _, _, A, _, W, END,
            W, _, _, X, _, W, END,
            W, _, X, _, W, END,
            W, _, _, W, END,
            W, W, W, END,
            END,
        },
        .message =
        "Welcome back to 2024",
    },
    {
        .data = (const uint8_t[]){
            W, W, W, W, W, W, W, W, END,
            W, _, _, _, _, _, X, W, END,
            W, _, _, W, W, _, W, W, END,
            W, _, R, A, L, _, W, END,
            W, W, _, W, B, _, W, END,
            W, _, P, _, _, _, W, END,
            W, _, B, X, _, W, W, END,
            W, _, _, W, W, W, END,
            W, W, W, W, END,
            END,
        },
    },
    {
        .data = (const uint8_t[]){
            _, _, W, W, W, W, W, END,
            _, _, W, X, X, X, W, END,
            W, W, W, _, _, _, W, W, W, END,
            W, X, _, R, _, D, _, X, W, END,
            W, X, _, _, A, _, _, X, W, END,
            W, X, _, U, _, L, _, X, W, END,
            W, W, W, _, P, _, W, W, W, END,
            _, _, W, X, X, X, W, END,
            _, _, W, W, W, W, W, END,
            END,
        },
    },
    {
        .data = (const uint8_t[]){
            W, W, W, W, W, W, W, W, W, END,
            W, _, _, _, W, _, _, _, W, END,
            W, _, X, _, W, _, X, _, W, END,
            W, W, W, _, W, _, W, W, W, END,
            _, W, _, _, X, _, W, W, END,
            _, W, _, X, _, X, _, W, END,
            _, W, _, W, R, W, _, W, END,
            _, W, _, _, _, _, _, W, END,
            _, W, W, X, W, A, _, W, END,
            _, _, W, _, _, U, X, W, END,
            _, _, W, W, _, P, _, W, END,
            _, _, _, W, W, W, W, W, END,
            END,
        },
    },
    {
        .data = (const uint8_t[]){
            W, W, W, W, W, W, W, END,
            W, _, A, _, A, _, W, END,
            W, U, _, L, _, U, W, END,
            W, _, U, _, X, _, W, END,
            W, X, _, U, _, L, W, END,
            W, _, P, _, X, _, W, END,
            W, P, _, R, _, U, W, END,
            W, W, W, W, W, W, W, END,
            END,
        },
    },
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,_,A,_,_,_,W,W, END,
			W,_,_,D,_,_,W,W, END,
			W,_,_,_,U,X,_,W, END,
			W,X,P,_,_,B,_,W, END,
			W,W,W,_,L,R,_,W, END,
			W,W,W,W,_,X,_,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,W,W,W,_,U,W,W, END,
			W,W,W,_,A,_,_,W, END,
			W,_,X,_,_,_,_,W, END,
			W,D,_,_,B,_,_,W, END,
			W,_,_,_,L,R,_,W, END,
			W,W,W,X,_,X,P,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,W,X,W,W,W,W,W, END,
			W,X,_,W,W,W,W,W, END,
			W,_,_,_,X,_,_,W, END,
			W,_,_,_,R,B,_,W, END,
			W,U,L,P,_,D,_,W, END,
			W,A,_,_,_,_,W,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,_,_,A,_,W,W,W, END,
			W,_,_,P,_,X,D,W, END,
			W,_,_,_,X,_,_,W, END,
			W,_,X,R,_,D,_,W, END,
			W,U,_,_,_,X,_,W, END,
			W,U,_,_,_,_,R,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,X,_,L,X,W,W,W, END,
			W,L,X,_,B,_,_,W, END,
			W,_,_,A,_,D,_,W, END,
			W,W,_,U,X,X,W,W, END,
			W,W,U,D,_,P,_,W, END,
			W,W,_,X,_,_,B,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,W,W,W,W,W,W,W, END,
			W,W,W,W,W,W,W,W, END,
			W,B,X,D,A,W,W,W, END,
			W,_,_,R,_,W,W,W, END,
			W,_,X,U,_,P,_,W, END,
			W,_,P,R,P,_,_,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,_,_,_,W,W,W,W, END,
			W,U,_,A,W,_,_,W, END,
			W,_,_,_,X,_,_,W, END,
			W,D,P,L,_,P,D,W, END,
			W,W,_,_,U,_,_,W, END,
			W,W,A,L,W,W,W,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			W,W,W,W,W,W,W,W, END,
			W,_,U,P,W,W,W,W, END,
			W,U,B,_,W,W,W,W, END,
			W,W,W,A,_,L,X,W, END,
			W,W,X,R,X,_,X,W, END,
			W,W,P,D,X,L,P,W, END,
			W,W,W,X,_,_,R,W, END,
			W,W,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			_,W,W,W,W,W,W, END,
			W,_,D,_,_,P,X,W, END,
			W,_,_,_,P,_,X,W, END,
			W,_,U,X,_,A,_,W, END,
			_,W,X,_,X,P,W, END,
			_,_,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			_,_,W,W,W,W, END,
			_,W,_,_,_,_,W, END,
			W,A,_,_,_,X,_,W, END,
			W,_,L,_,_,_,_,W, END,
			W,A,P,D,X,L,_,W, END,
			W,D,_,X,R,X,_,W, END,
			_,W,W,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			_,W,W,W,W, END,
			W,_,_,_,L,W, END,
			W,U,X,_,_,W, END,
			W,X,_,X,A,W, END,
			W,_,P,_,_,W, END,
			W,X,L,_,_,W, END,
			W,_,_,_,_,W, END,
			W,X,_,_,U,W, END,
			_,W,W,W,W, END,
			END, END,
		},
	},
	{
		.data = (const uint8_t[]){
			_,_,_,_,_,_,W, END,
			_,W,W,W,W,W,_,W, END,
			W,_,X,_,B,_,_,_,W, END,
			W,X,_,D,_,P,_,_,W, END,
			W,A,_,_,_,L,_,_,W, END,
			_,W,W,W,_,R,_,_,W, END,
			_,_,_,_,W,W,W,W, END,
			END, END,
		},
	},
    {
        .data = (const uint8_t[]){
            _,_,_,_,_,W,W, END,
            _,_,W,W,W,_,_,W, END,
            _,W,X,X,_,U,_,W, END,
            W,R,_,X,A,P,_,W, END,
            _,W,A,_,X,_,U,W, END,
            W,_,_,_,_,_,W, END,
            W,R,P,W,_,X,W, END,
            W,_,_,W,W,W, END,
            _,W,W, END,
            END, END,
        },
    },
    {
        .data = (const uint8_t[]){
            _,_,W,W,W,W, END,
            _,W,_,P,_,_,W, END,
            W,A,D,_,_,L,W, END,
            W,_,_,_,U,_,W, END,
            W,X,X,P,P,R,W, END,
            _,W,W,W,W,W, END,
            END, END,
        },
    },
    {
        .data = (const uint8_t[]){
            _,W,W,W,W,W,W, END,
            W,_,P,_,_,_,X,W, END,
            W,X,D,U,_,_,D,W, END,
            W,X,A,X,B,_,_,W, END,
            W,_,B,_,_,_,_,W, END,
            W,X,W,W,W,W,W, END,
            _,W, END,
            END, END,
        },
    },
};

/* clang-format on */

const unsigned int nstages = sizeof(stages) / sizeof(stages[0]);
