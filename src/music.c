#include "wasm4.h"

struct note {
        int8_t num; /* midi note number (0-127), or rest (-1) */
        uint8_t length;
        uint8_t flags;
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

static const struct note notes[] = {
        NOTE(60, 8), NOTE(61, 8), NOTE(60, 4), NOTE(55, 8), NOTE(56, 8),
        NOTE(55, 4),

        NOTE(60, 8), NOTE(61, 8), NOTE(60, 8), NOTE(60, 8), NOTE(55, 4),
        NOTE(-1, 4),

        NOTE(60, 4), NOTE(60, 4), NOTE(55, 8), NOTE(55, 8), NOTE(55, 4),

        NOTE(60, 4), NOTE(60, 8), NOTE(60, 8), NOTE(55, 4), NOTE(-1, 4),

        GOTO(0),
};

static const unsigned int frames_per_measure = 96;

static unsigned int curnote_idx = 0;
static unsigned int curframe;
static unsigned int curnote_nframes;

void
music_update(void)
{
next:
        if (curframe == 0) {
                const struct note *cur = &notes[curnote_idx];
                if ((cur->flags & NOTE_FLAG_GOTO)) {
                        curnote_idx = cur->length;
                        goto next;
                }
                curnote_nframes = frames_per_measure / cur->length;
                if (cur->num != -1) {
                        tracef("tone %d %d", cur->num, curnote_nframes);
                        tone((uint8_t)cur->num, curnote_nframes << 8, 32,
                             TONE_PULSE1 | TONE_NOTE_MODE);
                }
        }
        curframe++;
        if (curframe == curnote_nframes) {
                curnote_idx++;
                curframe = 0;
        }
}
