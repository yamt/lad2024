#include <string.h>

#if defined(USE_CRANS)
#include "crans_decode.h"
#else
#include "chuff_decode.h"
#endif
#include "defs.h"
#include "hloader.h"
#include "hstages.h"
#include "rule.h"

void
decode_huff_stage(uint32_t stage_number, map_t map, struct map_info *info)
{
        static char msgbuf[MSG_SIZE_MAX];
        const struct hstage *stage = &packed_stages[stage_number];

        uint16_t offset = stage->data_offset;
        const bool has_message = (offset & HSTAGE_HAS_MESSAGE) != 0;
        offset &= ~HSTAGE_HAS_MESSAGE;

#if defined(USE_CRANS)
        struct crans_decode_context ctx;
        ctx.inp = &stages_huff_data[offset];
#else
        struct chuff_decode_context ctx;
        bitin_init(&ctx.in, &stages_huff_data[offset]);
#endif
        ctx.ctx = 0;

        memset(map, 0, map_height * map_width);
        int x;
        int y;
        x = y = 0;
        int xmax = 0;
        uint8_t ch;
        while ((ch =
#if defined(USE_CRANS)
                        crans_decode_byte(&ctx, stages_huff_table, NULL,
                                          stages_huff_table_idx)
#else
                        chuff_decode_byte(&ctx, stages_huff_table,
                                          stages_huff_table_idx)
#endif
                        ) != END) {
                do {
                        map[genloc(x, y)] = ch;
                        x++;
                        if (xmax < x) {
                                xmax = x;
                        }
                } while (
                        (ch =
#if defined(USE_CRANS)
                                 crans_decode_byte(&ctx, stages_huff_table,
                                                   NULL, stages_huff_table_idx)
#else
                                 chuff_decode_byte(&ctx, stages_huff_table,
                                                   stages_huff_table_idx)
#endif
                                 ) != END);
                x = 0;
                y++;
        }
        info->w = (unsigned int)xmax;
        info->h = (unsigned int)y;

        if (has_message) {
                ctx.ctx = 0;
                char *p = msgbuf;
                char ch;
                do {
                        *p++ = ch = (char)
#if defined(USE_CRANS)
                                crans_decode_byte(&ctx, stages_msg_huff_table,
                                                  stages_msg_trans,
                                                  stages_msg_huff_table_idx);
#else
                                chuff_decode_byte(&ctx, stages_msg_huff_table,
                                                  stages_msg_huff_table_idx);
#endif
                } while (ch != 0);
                info->message = msgbuf;
        } else {
                info->message = NULL;
        }
}
