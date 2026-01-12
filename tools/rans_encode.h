#if !defined(_RANS_ENCODE_H_)
#define _RANS_ENCODE_H_

#include "rans_param.h"

struct rans_encode_state {
        rans_I x;
};

void rans_encode_init(struct rans_encode_state *st);

struct byteout;
void rans_encode_sym(struct rans_encode_state *st, rans_sym_t sym,
                     rans_prob_t b_s, rans_prob_t l_s, struct byteout *bo);
void rans_encode_flush(struct rans_encode_state *st, struct byteout *bo);

#endif /* !defined(_RANS_ENCODE_H_) */
