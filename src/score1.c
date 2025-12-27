#include "wasm4.h"

#include "scoredefs.h"

const struct score score1 = {
        .frames_per_measure = 96,
        .nparts = 2,
        /* clang-format off */
        .parts = {
                PART(0, TONE_PULSE1,
                        NOTE(60, 8),
                        NOTE(61, 8),
                        NOTE(60, 4),
                        NOTE(55, 8),
                        NOTE(56, 8),
                        NOTE(55, 4),

                        NOTE(60, 8),
                        NOTE(61, 8),
                        NOTE(60, 8),
                        NOTE(60, 8),
                        NOTE(55, 4),
                        NOTE(-1, 4),

                        NOTE(60, 4),
                        NOTE(60, 4),
                        NOTE(55, 8),
                        NOTE(55, 8),
                        NOTE(55, 4),

                        NOTE(60, 4),
                        NOTE(60, 8),
                        NOTE(60, 8),
                        NOTE(55, 4),
                        NOTE(-1, 4),

                        GOTO(0),
                ),
                PART(1, TONE_PULSE2,
                        NOTE(-1, 1),

                        NOTE(-1, 1),

                        NOTE(57, 4),
                        NOTE(57, 4),
                        NOTE(55, 8),
                        NOTE(52, 8),
                        NOTE(52, 4),

                        NOTE(57, 4),
                        NOTE(57, 8),
                        NOTE(57, 8),
                        NOTE(52, 4),
                        NOTE(-1, 4),

                        GOTO(0),
                ),
        },
        /* clang-format on */
};
