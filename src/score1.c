#include "wasm4.h"

#include "scoredefs.h"

const struct score score1 = {
        .frames_per_measure = 96,
        .nparts = 2,
        /* clang-format off */
        .parts = {
                PART(0, TONE_PULSE1,
                        NOTE(C4, 8),
                        NOTE(D4, 8),
                        NOTE(C4, 4),
                        NOTE(G3, 8),
                        NOTE(A3, 8),
                        NOTE(G3, 4),

                        NOTE(C4, 8),
                        NOTE(D4, 8),
                        NOTE(C4, 8),
                        NOTE(C4, 8),
                        NOTE(G3, 4),
                        NOTE(-1, 4),

                        NOTE(C4, 4),
                        NOTE(C4, 4),
                        NOTE(G3, 8),
                        NOTE(G3, 8),
                        NOTE(G3, 4),

                        NOTE(C4, 4),
                        NOTE(C4, 8),
                        NOTE(C4, 8),
                        NOTE(G3, 4),
                        NOTE(-1, 4),

                        GOTO_NTIMES(0, 1),

                        NOTE(E4, 8),
                        NOTE(F4, 8),
                        NOTE(E4, 4),
                        NOTE(B3, 8),
                        NOTE(C4, 8),
                        NOTE(B3, 4),

                        NOTE(F4, 8),
                        NOTE(G4, 8),
                        NOTE(F4, 4),
                        NOTE(B3, 8),
                        NOTE(C4, 8),
                        NOTE(B3, 4),

                        NOTE(G4, 8),
                        NOTE(A4, 8),
                        NOTE(G4, 4),
                        NOTE(D4, 8),
                        NOTE(E4, 8),
                        NOTE(D4, 4),

                        NOTE(A4, 8),
                        NOTE(B4, 8),
                        NOTE(A4, 4),
                        NOTE(D4, 8),
                        NOTE(E4, 8),
                        NOTE(D4, 4),

                        NOTE(G4, 8),
                        NOTE(A4, 8),
                        NOTE(G4, 4),
                        NOTE(D4, 8),
                        NOTE(E4, 8),
                        NOTE(D4, 4),

                        NOTE(A4, 8),
                        NOTE(B4, 8),
                        NOTE(A4, 4),
                        NOTE(E4, 8),
                        NOTE(F4, 8),
                        NOTE(E4, 4),

                        NOTE(B4, 8),
                        NOTE(C5, 8),
                        NOTE(B4, 4),
                        NOTE(E4, 8),
                        NOTE(F4, 8),
                        NOTE(E4, 4),

                        NOTE(C5, 8),
                        NOTE(D5, 8),
                        NOTE(C5, 4),
                        NOTE(E4, 8),
                        NOTE(F4, 8),
                        NOTE(E4, 4),

                        GOTO(0),
                ),
                PART(1, TONE_PULSE2,
                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(A3, 4),
                        NOTE(A3, 4),
                        NOTE(E3, 8),
                        NOTE(E3, 8),
                        NOTE(E3, 4),

                        NOTE(A3, 4),
                        NOTE(A3, 8),
                        NOTE(A3, 8),
                        NOTE(E3, 4),
                        NOTE(-1, 4),

                        GOTO_NTIMES(0, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        GOTO(0),
                ),
        },
        /* clang-format on */
};
