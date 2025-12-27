#include <string.h>

#include "wasm4.h"

#include "scoredefs.h"
#include "scores.h"

static const struct score *curscore;

struct part_state {
        unsigned int curnote_idx;
        unsigned int curframe;
        unsigned int curnote_nframes;
        unsigned int volume;
} part_state[MAX_PARTS];

static void
part_init(struct part_state *state)
{
        memset(state, 0, sizeof(*state));
        state->volume = DEFAULT_VOLUME;
}

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
                if ((cur->flags & NOTE_FLAG_DYN)) {
                        state->volume = cur->length;
                        state->curnote_idx++;
                        goto next;
                }
                state->curnote_nframes =
                        score->frames_per_measure / cur->length;
                if (cur->num != -1) {
                        tracef("tone %d %d", cur->num, state->curnote_nframes);
                        tone((uint8_t)cur->num, state->curnote_nframes << 8,
                             state->volume, part->channel | TONE_NOTE_MODE);
                }
        }

        state->curframe++;
        if (state->curframe == state->curnote_nframes) {
                state->curnote_idx++;
                state->curframe = 0;
        }
}

void
music_change(const struct score *score)
{
        if (curscore == score) {
                return;
        }
        unsigned int i;
        for (i = 0; i < MAX_PARTS; i++) {
                part_init(&part_state[i]);
        }
        curscore = score;
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
