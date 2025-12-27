#include "wasm4.h"

#include "scoredefs.h"
#include "scores.h"

static const struct score *curscore = &score2;

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
                             32, part->channel | TONE_NOTE_MODE);
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
