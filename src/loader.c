#include <string.h>

#include "defs.h"
#include "rule.h"
#include "loader.h"
#include "stages.h"

void
decode_stage(uint32_t stage_number, map_t map, struct map_info *info)
{
        const struct stage *stage = &stages[stage_number];

        memset(map, 0, height * width);
        int x;
        int y;
        x = y = 0;
        int xmax = 0;
        const uint8_t *p = stage->data;
        uint8_t ch;
        while ((ch = *p++) != END) {
                do {
                        map[genloc(x, y)] = ch;
                        x++;
                        if (xmax < x) {
                                xmax = x;
                        }
                } while ((ch = *p++) != END);
                x = 0;
                y++;
        }
        info->w = xmax;
        info->h = y;
        info->message = stage->message;
}
