#include "rule.h"

struct map_info {
        unsigned int w;
        unsigned int h;
        const char *message;
};

void decode_stage(uint32_t stage_number, map_t map, struct map_info *info);
