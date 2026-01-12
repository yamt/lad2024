#include <stdint.h>

#if defined(USE_CRANS)
#include "rans_param.h"
#endif

struct hstage {
        const uint16_t data_offset;
};

#define HSTAGE_HAS_MESSAGE 0x8000

extern const uint8_t stages_huff_data[];
extern const uint16_t stages_huff_table_idx[];
#if defined(USE_CRANS)
extern const rans_prob_t stages_huff_table[];
extern const rans_prob_t stages_msg_huff_table[];
extern const rans_sym_t stages_msg_trans[];
#else
extern const uint8_t stages_huff_table[];
extern const uint8_t stages_msg_huff_table[];
#endif
extern const uint16_t stages_msg_huff_table_idx[];
extern const struct hstage packed_stages[];
