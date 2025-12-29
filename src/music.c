#include <stdbool.h>
#include <string.h>

#include "util.h"
#include "wasm4.h"
#include "wasm4util.h"

#include "scoredefs.h"
#include "scores.h"

static const struct score *curscore;
static const struct score *nextscore;
static uint8_t master_volume; /* 0-255 */
static bool fading;

struct part_state {
        unsigned int curnote_idx;
        uint8_t curframe;
        uint8_t curnote_nframes;
        uint8_t channel; /* channel, mode, pan */
        uint8_t volume;
        uint8_t tone;
        uint8_t ntimes_count;
} part_state[MAX_PARTS];

static void
part_init(struct part_state *state, const struct part *part)
{
        memset(state, 0, sizeof(*state));
        state->volume = DEFAULT_VOLUME;
        state->channel = part->channel;
        state->tone = part->tone;
}

static const struct tone {
        uint8_t a;
        uint8_t d;
        uint8_t s;
        uint8_t r;

        /* volume */
        uint8_t peak;
        uint8_t sustain;
} tones[] = {
        {0, 255, 0, 0, 255, 0},
        {128, 128, 0, 128, 255, 128},
};

static unsigned int
scale(unsigned int a, uint8_t b)
{
        return (a * (unsigned int)b) >> 8;
}

static void
play_note(uint8_t num, uint8_t len, uint8_t volume, uint8_t channel,
          const struct tone *t)
{
        ASSERT(volume <= 100);

        /*
         * REVISIT: probably it makes more sense to use constant values
         * rather than scale values. eg. A, D for for some kind of tones.
         */
        unsigned int a = scale(len, t->a);
        unsigned int d = scale(len, t->d);
        unsigned int s = scale(len, t->s);
        unsigned int r = scale(len, t->r);
        unsigned int volume_peak = scale(volume, t->peak);
        unsigned int volume_sustain = scale(volume, t->sustain);

        if (volume_peak == 0) {
                /*
                 * wasm4 tone() api takes peak=0 as peak=100,
                 * which is not our intention.
                 */
                volume_peak = 1;
        }

        tracef("num %d len %d (a %d d %d s %d r %d) volume %d "
               "(peak %d sustain %d)",
               num, len, a, d, s, r, volume, volume_peak, volume_sustain);
        ASSERT(a < 256);
        ASSERT(d < 256);
        ASSERT(s < 256);
        ASSERT(r < 256);
        ASSERT(volume_peak < 100);
        ASSERT(volume_sustain < 100);
        tone(num, TONE_DURATION(a, d, s, r),
             TONE_VOLUME(volume_peak, volume_sustain),
             channel | TONE_NOTE_MODE);
}

static void
part_update(const struct score *score, const struct part *part,
            struct part_state *state)
{
next:
        if (state->curframe == 0) {
                const struct note *cur = &part->notes[state->curnote_idx];
                if (cur->type == NOTE_TYPE_NTIMES) {
                        if (state->ntimes_count >= NOTE_VALUE(cur)) {
                                state->ntimes_count = 0;
                                state->curnote_idx += 2;
                                goto next;
                        }
                        state->ntimes_count++;
                        state->curnote_idx++;
                        goto next;
                }
                if (cur->type == NOTE_TYPE_GOTO) {
                        state->curnote_idx = NOTE_VALUE(cur);
                        goto next;
                }
                if (cur->type == NOTE_TYPE_DYN) {
                        state->volume = (uint8_t)NOTE_VALUE(cur);
                        state->curnote_idx++;
                        goto next;
                }
                if (cur->type == NOTE_TYPE_CHANNEL) {
                        state->channel = (uint8_t)NOTE_VALUE(cur);
                        state->curnote_idx++;
                        goto next;
                }
                if (cur->type == NOTE_TYPE_TONE) {
                        state->tone = (uint8_t)NOTE_VALUE(cur);
                        state->curnote_idx++;
                        goto next;
                }
                state->curnote_nframes =
                        score->frames_per_measure / cur->length;
                if (!NOTE_IS_REST(cur) && master_volume > 0) {
                        uint8_t num = NOTE_NUM(cur);
                        ASSERT(num <= 127);
                        uint8_t volume =
                                (uint8_t)scale(state->volume, master_volume);
                        const struct tone *t = &tones[state->tone];
                        play_note(num, state->curnote_nframes, volume,
                                  state->channel, t);
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
        for (i = 0; i < nextscore->nparts; i++) {
                part_init(&part_state[i], &nextscore->parts[i]);
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
                start_fading();
                /*
                 * short delay to avoid the audio hiccup seen
                 * on startup of wasm4 native runtime
                 */
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
