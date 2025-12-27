#include "wasm4.h"

#include "scoredefs.h"

const struct score score3 = {
        .frames_per_measure = 64,
        .nparts = 2,
        /* clang-format off */
        .parts = {
                PART(0, TONE_PULSE1,

                        /* 1 */

                        NOTE(A4, 4),
                        NOTE(C5, 4),
                        NOTE(D5, 4),
                        NOTE(C5, 4),

                        NOTE(A4, 4),
                        NOTE(G4, 4),
                        NOTE(A4, 2),

                        NOTE(A4, 4),
                        NOTE(C5, 4),
                        NOTE(D5, 4),
                        NOTE(C5, 4),

                        NOTE(E5, 4),
                        NOTE(D5, 4),
                        NOTE(E5, 2),

                        /* 5 */

                        NOTE(A4, 4),
                        NOTE(C5, 4),
                        NOTE(D5, 4),
                        NOTE(C5, 4),

                        NOTE(A4, 4),
                        NOTE(G4, 4),
                        NOTE(A4, 2),

                        NOTE(A4, 4),
                        NOTE(C5, 4),
                        NOTE(D5, 4),
                        NOTE(C5, 4),

                        NOTE(E5, 4),
                        NOTE(E5, 4),
                        NOTE(E5, 2),

                        /* 9 */

                        NOTE(E5, 4),
                        NOTE(G5, 4),
                        NOTE(F5, 4),
                        NOTE(E5, 4),

                        NOTE(D5, 4),
                        NOTE(F5, 4),
                        NOTE(E5, 4),
                        NOTE(D5, 4),

                        NOTE(C5, 4),
                        NOTE(E5, 4),
                        NOTE(D5, 4),
                        NOTE(C5, 4),

                        NOTE(B4, 1),

                        GOTO(0),
                ),
                PART(1, TONE_TRIANGLE,

                        /* 1 */

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        /* 5 */

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        NOTE(A3, 4),
                        NOTE(E4, 4),
                        NOTE(C4, 4),
                        NOTE(E4, 4),

                        /* 9 */

                        NOTE(E4, 1),

                        NOTE(D4, 1),

                        NOTE(C4, 1),

                        NOTE(B3, 4),
                        NOTE(D4, 4),
                        NOTE(C4, 4),
                        NOTE(B3, 4),

                        GOTO(0),
                ),
        },
        /* clang-format on */
};
