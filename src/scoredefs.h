struct note {
        uint8_t num;        /* midi note number (0-127), or rest (-1) */
        uint8_t length : 5; /* 0-31 */
        uint8_t type : 3;
};

struct part {
        uint8_t channel;
        uint8_t tone;
        const struct note *notes;
};

#define MAX_PARTS 2

struct score {
        uint8_t frames_per_measure;
        uint8_t nparts;
        const struct part parts[MAX_PARTS];
};

#define DEFAULT_VOLUME 24

#define NOTE_TYPE_GOTO 0x01
#define NOTE_TYPE_DYN 0x02
#define NOTE_TYPE_NTIMES 0x03
#define NOTE_TYPE_CHANNEL 0x04
#define NOTE_TYPE_TONE 0x05

/* get a 13-bit value (0-8191) */
#define NOTE_VALUE(n) (((uint32_t)(n)->length << 8) | (n)->num)
#define NOTE_HIGH(v) ((v) >> 8)
#define NOTE_LOW(v) ((v)&0xff)

#define NOTE_IS_REST(n) ((n)->num == (uint8_t)-1)
#define NOTE_NUM(n) ((n)->num)
#define NOTE_DEST(n) ((int8_t)(n)->num)

#define NOTE(n, s)                                                            \
        {                                                                     \
                (uint8_t) n, s, 0,                                            \
        }
#define NOTE_SPECIAL(t, n)                                                    \
        {                                                                     \
                NOTE_LOW(n), NOTE_HIGH(n), t,                                 \
        }
#define GOTO(n) NOTE_SPECIAL(NOTE_TYPE_GOTO, n)
#define NTIMES(n) NOTE_SPECIAL(NOTE_TYPE_NTIMES, n)
#define GOTO_NTIMES(n, count) NTIMES(count), GOTO(n)
#define DYN(n) NOTE_SPECIAL(NOTE_TYPE_DYN, n)
#define SET_CHANNEL(n) NOTE_SPECIAL(NOTE_TYPE_CHANNEL, n)
#define SET_TONE(n) NOTE_SPECIAL(NOTE_TYPE_TONE, n)

#define PART(part_no, channel_no, toneidx, ...)                               \
        [part_no] = {.channel = channel_no,                                   \
                     .tone = toneidx,                                         \
                     .notes = (const struct note[]){__VA_ARGS__}}

enum note_number {
        C3 = 48,
        C3SHARP,
        D3,
        D3SHARP,
        E3,
        F3,
        F3SHARP,
        G3,
        G3SHARP,
        A3,
        A3SHARP,
        B3,

        C4,
        C4SHARP,
        D4,
        D4SHARP,
        E4,
        F4,
        F4SHARP,
        G4,
        G4SHARP,
        A4,
        A4SHARP,
        B4,

        C5,
        C5SHARP,
        D5,
        D5SHARP,
        E5,
        F5,
        F5SHARP,
        G5,
        G5SHARP,
        A5,
        A5SHARP,
        B5,
};
