#include "wasm4.h"

struct note {
        int8_t num; /* midi note number (0-127), or rest (-1) */
        uint8_t length;
        uint8_t flags;
};

struct part {
        const struct note *notes;
};

struct score {
        unsigned int frames_per_measure;
        unsigned int nparts;
        const struct part parts[2];
};

#define NOTE_FLAG_GOTO 0x01

#define NOTE(n, s)                                                            \
        {                                                                     \
                n, s, 0                                                       \
        }
#define GOTO(n)                                                               \
        {                                                                     \
                0, n, NOTE_FLAG_GOTO                                          \
        }

#define PART(part_no, ...)                                                    \
        [part_no] = {.notes = (const struct note[]){__VA_ARGS__}}

static const struct score score1 = {
        .frames_per_measure = 96,
        .nparts = 1,
        /* clang-format off */
        .parts = {
                PART(0,
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
        },
        /* clang-format on */
};

static const struct score *curscore = &score1;

struct part_state {
        unsigned int curnote_idx;
        unsigned int curframe;
        unsigned int curnote_nframes;
} part_state[2];

static void
part_update(const struct score *score, const struct part *part,
            struct part_state *state)
{
next:
        if (state->curframe == 0) {
                const struct note *cur = &part->notes[state->curnote_idx];
                if ((cur->flags & NOTE_FLAG_GOTO)) {
                        state->curnote_idx = cur->length;
                        goto next;
                }
                state->curnote_nframes =
                        score->frames_per_measure / cur->length;
                if (cur->num != -1) {
                        tracef("tone %d %d", cur->num, state->curnote_nframes);
                        tone((uint8_t)cur->num, state->curnote_nframes << 8,
                             32, TONE_PULSE1 | TONE_NOTE_MODE);
                }
        }

        state->curframe++;
        if (state->curframe == state->curnote_nframes) {
                state->curnote_idx++;
                state->curframe = 0;
        }
}

void
music_update(void)
{
        unsigned int i;
        for (i = 0; i < curscore->nparts; i++) {
                const struct part *part = &curscore->parts[i];
                part_update(curscore, part, &part_state[i]);
        }
}
