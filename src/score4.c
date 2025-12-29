#include "wasm4.h"

#include "scoredefs.h"

/*
 * iwau uta
 */

/* clang-format off */

const struct score score4 = {
        .frames_per_measure = 72,
        .nparts = 2,
        .parts = {
                PART(0, TONE_PULSE1, 1,

                        /* 1 */

                        NOTE(-1, 4),
                        NOTE(-1, 4),
                        NOTE(F4, 4),
                        NOTE(F4, 4),

                        NOTE(E4, 4),
                        NOTE(D4, 4),
                        NOTE(C4, 2),

                        NOTE(D4, 4),
                        NOTE(D4, 4),
                        NOTE(E4, 4),
                        NOTE(E4, 4),

                        NOTE(F4, 4),
                        NOTE(F4, 4),
                        NOTE(G4, 4),
                        NOTE(G4, 4),

                        /* 5 */

                        NOTE(A4, 4),
                        NOTE(A4, 4),
                        NOTE(B4, 4),
                        NOTE(B4, 4),

                        NOTE(C5, 1),

                        /* 7 */

                        NOTE(G4, 2),
                        NOTE(G4, 2),

                        NOTE(G4, 2),
                        NOTE(C5, 1),

                        NOTE(-1, 2),

                        /* 10 */

                        NOTE(A4, 4),
                        NOTE(A4, 4),
                        NOTE(A4, 4),
                        NOTE(A4, 4),

                        NOTE(A4, 2),
                        NOTE(A4, 4),
                        NOTE(A4, 4),

                        NOTE(G4, 4),
                        NOTE(G4, 4),
                        NOTE(G4, 4),
                        NOTE(G4, 4),

                        NOTE(G4, 2),
                        NOTE(G4, 4),
                        NOTE(G4, 4),

                        /* 14 */

                        NOTE(F4, 4),
                        NOTE(F4, 4),
                        NOTE(F4, 4),
                        NOTE(F4, 4),

                        NOTE(E4, 2),
                        NOTE(D4, 2),

                        NOTE(C4, 2),
                        NOTE(-1, 2),

                        NOTE(-1, 1),

                        GOTO_NTIMES(25, 7), /* jump to 10, repeat 8 times */

                        /* 18 */

                        NOTE(-1, 2),
                        NOTE(C5, 4),
                        NOTE(A4, 4),

                        NOTE(G4, 4),
                        NOTE(F4, 4),
                        NOTE(-1, 2),

                        GOTO(20), /* jump to 7 */
                ),
                PART(1, TONE_PULSE2, 1,

                        /* 1 */

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        /* 7 */

                        DYN(DEFAULT_VOLUME),

                        NOTE(G3, 2),
                        NOTE(G3, 2),

                        NOTE(G3, 2),
                        NOTE(G3, 1),

                        NOTE(-1, 2),

                        /* 10 */

                        DYN(DEFAULT_VOLUME / 3 * 2),
                        SET_TONE(0),

                        NOTE(A3, 4),
                        NOTE(C4, 4),
                        NOTE(A3, 4),
                        NOTE(C4, 4),

                        NOTE(A3, 4),
                        NOTE(C4, 4),
                        NOTE(A3, 4),
                        NOTE(C4, 4),

                        NOTE(G3, 4),
                        NOTE(C4, 4),
                        NOTE(G3, 4),
                        NOTE(C4, 4),

                        NOTE(G3, 4),
                        NOTE(C4, 4),
                        NOTE(G3, 4),
                        NOTE(C4, 4),

                        /* 14 */

                        NOTE(F3, 4),
                        NOTE(A3, 4),
                        NOTE(F3, 4),
                        NOTE(A3, 4),

                        NOTE(F3, 4),
                        NOTE(A3, 4),
                        NOTE(F3, 4),
                        NOTE(A3, 4),

                        NOTE(E3, 4),
                        NOTE(G3, 4),
                        NOTE(E3, 4),
                        NOTE(G3, 4),

                        NOTE(E3, 4),
                        NOTE(G3, 4),
                        NOTE(E3, 4),
                        NOTE(G3, 4),

                        SET_TONE(1),
                        GOTO_NTIMES(12, 7), /* jump to 10, repeat 8 times */

                        /* 18 */

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        GOTO(6), /* jump to 7 */
                ),
        },
};

/* clang-format on */
