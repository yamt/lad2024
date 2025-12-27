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

#define DEFAULT_VOLUME 32

#define NOTE_FLAG_GOTO 0x01
#define NOTE_FLAG_DYN 0x02

#define NOTE(n, s)                                                            \
        {                                                                     \
                n, s, 0                                                       \
        }
#define GOTO(n)                                                               \
        {                                                                     \
                0, n, NOTE_FLAG_GOTO                                          \
        }
#define DYN(n)                                                                \
        {                                                                     \
                0, n, NOTE_FLAG_DYN                                           \
        }

#define PART(part_no, channel_no, ...)                                        \
        [part_no] = {.channel = channel_no,                                   \
                     .notes = (const struct note[]){__VA_ARGS__}}
