/*
 * https://wasm4.org/docs/reference/functions#tone-frequency-duration-volume-flags
 * https://wasm4.org/docs/guides/audio
 */
#define TONE_FREQ(start_freq, end_freq) ((start_freq) | ((end_freq) << 16))
#define TONE_NOTE_FREQ(note, bend) ((note) | ((bend) << 8))
#define TONE_DURATION(sustain, release, decay, attack)                        \
        ((sustain) | ((release) << 8) | ((decay) << 16) | ((attack) << 24))
#define TONE_VOLUME(sustain_volume, peak_volume)                              \
        ((sustain_volume) | ((peak_volume) << 8))
#define TONE_FLAGS(channel, mode, pan, note)                                  \
        ((channel) | ((mode) << 2) | ((pan) << 4) | (note))
