#include <string.h>

#include "defs.h"
#include "hloader.h"
#include "hstages.h"
#include "huff_decode.h"
#include "rule.h"

void
decode_huff_stage(uint32_t stage_number, map_t map, struct map_info *info)
{
        const struct hstage *stage = &packed_stages[stage_number];
        struct huff_decode_context ctx;

        huff_decode_init(&ctx, &stages_huff_data[stage->data_offset]);
        memset(map, 0, map_height * map_width);
        int x;
        int y;
        x = y = 0;
        int xmax = 0;
        uint8_t ch;
        while ((ch = huff_decode_byte(&ctx, stages_huff_table)) != END) {
                do {
                        map[genloc(x, y)] = ch;
                        x++;
                        if (xmax < x) {
                                xmax = x;
                        }
                } while ((ch = huff_decode_byte(&ctx, stages_huff_table)) !=
                         END);
                x = 0;
                y++;
        }
        info->w = (unsigned int)xmax;
        info->h = (unsigned int)y;
        info->message = (const char *)stage->message;
}
