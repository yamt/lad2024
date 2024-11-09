#include <stdint.h>

struct hstage {
        const unsigned int data_offset;
        const uint8_t *message;
};

extern const uint8_t stages_huff_data[];
extern const uint8_t stages_huff_table[];
extern const struct hstage packed_stages[];
