#include "rans_param.h"

struct rans_encode_state {
        I x;
};

void rans_encode_init(struct rans_encode_state *st);

struct byteout;
void rans_encode_sym(struct rans_encode_state *st, sym_t sym, prob_t c_sym,
                     prob_t p_sym, struct byteout *bo);
void rans_encode_flush(struct rans_encode_state *st, struct byteout *bo);
