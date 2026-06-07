#include <stdint.h>

#define max_stages 1400
#define save_data_version 1
struct save_data {
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t clear_bitmap[(max_stages + 32 - 1) / 32];
};

extern struct save_data state;

void load_state(void);
void save_state(void);
