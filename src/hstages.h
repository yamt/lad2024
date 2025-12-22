#include <stdint.h>

struct hstage {
        const uint16_t data_offset;
};

#define HSTAGE_HAS_MESSAGE 0x8000

extern const uint8_t stages_huff_data[];
extern const uint8_t stages_huff_table[];
extern const uint16_t stages_huff_table_idx[];
extern const uint8_t stages_msg_huff_table[];
extern const uint16_t stages_msg_huff_table_idx[];
extern const struct hstage packed_stages[];
