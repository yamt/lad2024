struct note {
        int8_t num; /* midi note number (0-127), or rest (-1) */
        uint8_t length;
        uint8_t flags;
};

struct part {
        unsigned int channel;
        const struct note *notes;
};

#define MAX_PARTS 2

struct score {
        unsigned int frames_per_measure;
        unsigned int nparts;
        const struct part parts[MAX_PARTS];
};

#define DEFAULT_VOLUME 24

#define NOTE_FLAG_GOTO 0x01
#define NOTE_FLAG_DYN 0x02
#define NOTE_FLAG_NTIMES 0x04

#define NOTE(n, s)                                                            \
        {                                                                     \
                n, s, 0                                                       \
        }
#define GOTO(n)                                                               \
        {                                                                     \
                0, n, NOTE_FLAG_GOTO                                          \
        }
#define GOTO_NTIMES(n, count)                                                 \
        {                                                                     \
                count, n, NOTE_FLAG_NTIMES | NOTE_FLAG_GOTO                   \
        }
#define DYN(n)                                                                \
        {                                                                     \
                0, n, NOTE_FLAG_DYN                                           \
        }

#define PART(part_no, channel_no, ...)                                        \
        [part_no] = {.channel = channel_no,                                   \
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
