#include <stdbool.h>

#include "rans_param.h"

struct rans_decode_state {
        rans_I x;
};

void rans_decode_init(struct rans_decode_state *st);
rans_sym_t rans_decode_sym(struct rans_decode_state *st,
                           const rans_prob_t ls[RANS_NSYMS],
                           const uint8_t **inpp);
