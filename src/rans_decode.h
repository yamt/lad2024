#include <stdbool.h>

#include "rans_param.h"

struct rans_decode_state {
        I x;
};

void rans_decode_init(struct rans_decode_state *st);
sym_t rans_decode_sym(struct rans_decode_state *st, const prob_t ls[NSYMS],
                      const uint8_t **inpp);
