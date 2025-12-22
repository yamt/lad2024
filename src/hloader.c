#include <string.h>

#include "defs.h"
#include "hloader.h"
#include "hstages.h"
#include "huff_decode.h"
#include "rule.h"

struct chuff_decode_context {
        struct huff_decode_context hctx;
        uint8_t chuff_ctx;
};

static uint8_t
chuff_decode_byte(struct chuff_decode_context *ctx, const uint8_t *tbl,
                  const uint16_t *tblidx)
{
        ctx->chuff_ctx =
                huff_decode_byte(&ctx->hctx, &tbl[tblidx[ctx->chuff_ctx]]);
        return ctx->chuff_ctx;
}

void
decode_huff_stage(uint32_t stage_number, map_t map, struct map_info *info)
{
        static char msgbuf[140];
        const struct hstage *stage = &packed_stages[stage_number];
        struct chuff_decode_context ctx;

        uint16_t offset = stage->data_offset;
        const bool has_message = (offset & HSTAGE_HAS_MESSAGE) != 0;
        offset &= ~HSTAGE_HAS_MESSAGE;

        huff_decode_init(&ctx.hctx, &stages_huff_data[offset]);
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
