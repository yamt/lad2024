#include <stdint.h>

struct hstage {
        const uint16_t data_offset;
        const uint16_t msg_offset;
};

extern const uint8_t stages_huff_data[];
extern const uint8_t stages_huff_table[];
extern const uint16_t stages_huff_table_idx[];
extern const uint8_t stages_msg_huff_table[];
extern const uint16_t stages_msg_huff_table_idx[];
extern const struct hstage packed_stages[];
