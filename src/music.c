#include <stdbool.h>
#include <string.h>

#include "wasm4.h"
#include "wasm4util.h"

#include "scoredefs.h"
#include "scores.h"

static const struct score *curscore;
static const struct score *nextscore;
static unsigned int master_volume;
static bool fading;

struct part_state {
        unsigned int curnote_idx;
        unsigned int curframe;
        unsigned int curnote_nframes;
        unsigned int volume;
        unsigned int ntimes_count;
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
                if ((cur->flags & NOTE_FLAG_NTIMES) != 0) {
                        // tracef("ntimes %d/%d", state->ntimes_count,
                        // cur->num);
                        if (state->ntimes_count >= (unsigned int)cur->num) {
                                state->ntimes_count = 0;
                                state->curnote_idx++;
                                goto next;
                        }
                        state->ntimes_count++;
                }
                if ((cur->flags & NOTE_FLAG_GOTO) != 0) {
                        state->curnote_idx = cur->length;
                        goto next;
                }
                if ((cur->flags & NOTE_FLAG_DYN) != 0) {
                        state->volume = cur->length;
                        state->curnote_idx++;
                        goto next;
                }
                state->curnote_nframes =
                        score->frames_per_measure / cur->length;
                if (cur->num != -1) {
                        unsigned int volume = state->volume;
                        if (master_volume != 256) {
                                volume = (volume * master_volume) >> 8;
                        }
                        // tracef("tone %d %d", cur->num,
                        // state->curnote_nframes);
                        tone((uint8_t)cur->num, state->curnote_nframes << 8,
                             volume, part->channel | TONE_NOTE_MODE);
                }
        }

        state->curframe++;
        if (state->curframe == state->curnote_nframes) {
                state->curnote_idx++;
                state->curframe = 0;
        }
}

static void
start_next_score(void)
{
        unsigned int i;
        for (i = 0; i < MAX_PARTS; i++) {
                part_init(&part_state[i]);
        }
        master_volume = 255;
        curscore = nextscore;
}

static void
start_fading(void)
{
        fading = true;
}

void
music_change(const struct score *score)
{
        if (nextscore == score) {
                return;
        }
        nextscore = score;
        if (curscore == NULL) {
                // start_next_score();
                /*
                 * short delay to avoid the audio hiccup seen
                 * on startup of wasm4 native runtime
                 */
                start_fading();
                master_volume = 8;
        } else {
                start_fading();
        }
}

void
music_update(void)
{
        if (fading) {
                if (master_volume == 0) {
                        fading = false;
                        start_next_score();
                } else {
                        master_volume--;
                }
        }
        if (curscore == NULL) {
                return;
        }
        unsigned int i;
        for (i = 0; i < curscore->nparts; i++) {
                const struct part *part = &curscore->parts[i];
                part_update(curscore, part, &part_state[i]);
        }
}
