#include "wasm4.h"

#include "scoredefs.h"

const struct score score2 = {
        .frames_per_measure = 64,
        .nparts = 2,
        /* clang-format off */
        .parts = {
                PART(0, TONE_PULSE1,
                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(-1, 4),
                        NOTE(C4, 4),
                        NOTE(D4, 4),
                        NOTE(E4, 4),

                        NOTE(F4, 4),
                        NOTE(F4, 2),
                        NOTE(F4, 4),

                        NOTE(C4, 4),
                        NOTE(C4, 2),
                        NOTE(C4, 4),

                        NOTE(F4, 4),
                        NOTE(F4, 2),
                        NOTE(F4, 4),

                        NOTE(C4, 4),
                        NOTE(C4, 2),
                        NOTE(C4, 4),

                        NOTE(F4, 4),
                        NOTE(F4, 2),
                        NOTE(F4, 4),

                        NOTE(D4, 4),
                        NOTE(C4, 4),
                        NOTE(D4, 4),
                        NOTE(F4, 4),

                        NOTE(-1, 1),

                        NOTE(C4, 4),
                        NOTE(D4, 4),
                        NOTE(E4, 4),
                        NOTE(F4, 4),

                        NOTE(G4, 4),
                        NOTE(G4, 2),
                        NOTE(G4, 4),

                        NOTE(D4, 4),
                        NOTE(D4, 2),
                        NOTE(D4, 4),

                        NOTE(G4, 4),
                        NOTE(G4, 2),
                        NOTE(G4, 4),

                        NOTE(D4, 4),
                        NOTE(D4, 2),
                        NOTE(D4, 4),

                        NOTE(G4, 4),
                        NOTE(G4, 2),
                        NOTE(G4, 4),

                        NOTE(D4, 4),
                        NOTE(C4, 4),
                        NOTE(D4, 4),
                        NOTE(F4, 4),
                        GOTO(2),
                ),
                PART(1, TONE_PULSE2,
                        DYN(DEFAULT_VOLUME / 3 * 2),

                        NOTE(C3, 4),
                        NOTE(G3, 4),
                        NOTE(E3, 4),
                        NOTE(G3, 4),

                        GOTO(0),
                ),
        },
        /* clang-format on */
};
