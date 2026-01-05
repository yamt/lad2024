#include <string.h>

#include "chuff_decode.h"
#include "defs.h"
#include "hloader.h"
#include "hstages.h"
#include "rule.h"

void
decode_huff_stage(uint32_t stage_number, map_t map, struct map_info *info)
{
        static char msgbuf[MSG_SIZE_MAX];
        const struct hstage *stage = &packed_stages[stage_number];
        struct chuff_decode_context ctx;

        uint16_t offset = stage->data_offset;
        const bool has_message = (offset & HSTAGE_HAS_MESSAGE) != 0;
        offset &= ~HSTAGE_HAS_MESSAGE;

        bitin_init(&ctx.in, &stages_huff_data[offset]);
        ctx.chuff_ctx = 0;
        memset(map, 0, map_height * map_width);
        int x;
        int y;
        x = y = 0;
        int xmax = 0;
        uint8_t ch;
        while ((ch = chuff_decode_byte(&ctx, stages_huff_table,
                                       stages_huff_table_idx)) != END) {
                do {
                        map[genloc(x, y)] = ch;
                        x++;
                        if (xmax < x) {
                                xmax = x;
                        }
                } while ((ch = chuff_decode_byte(&ctx, stages_huff_table,
                                                 stages_huff_table_idx)) !=
                         END);
                x = 0;
                y++;
        }
        info->w = (unsigned int)xmax;
        info->h = (unsigned int)y;

        if (has_message) {
                ctx.chuff_ctx = 0;
                char *p = msgbuf;
                char ch;
                do {
                        *p++ = ch = (char)chuff_decode_byte(
                                &ctx, stages_msg_huff_table,
                                stages_msg_huff_table_idx);
                } while (ch != 0);
                info->message = msgbuf;
        } else {
                info->message = NULL;
        }
}
