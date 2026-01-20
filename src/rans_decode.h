#if !defined(_RANS_DECODE_H_)
#define _RANS_DECODE_H_

#include <stdbool.h>

#include "rans_param.h"

struct rans_decode_state {
        rans_I x;
};

void rans_decode_init(struct rans_decode_state *st);
bool rans_decode_need_more(const struct rans_decode_state *st);
void rans_decode_feed(struct rans_decode_state *st, uint16_t input);
rans_sym_t rans_decode_sym(struct rans_decode_state *st,
                           const rans_prob_t ls[RANS_NSYMS]);
rans_I rans_decode_get_extra(struct rans_decode_state *st);

#endif /* defined(_RANS_DECODE_H_) */
