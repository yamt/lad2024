#if !defined(_RANS_ENCODE_H_)
#define _RANS_ENCODE_H_

#include "rans_param.h"

struct rans_encode_state {
        rans_I x;
};

/*
 * state initialization
 * - rans_encode_init: initialize with I_MIN
 * - rans_encode_init_with_prob: initialize with the probability of
 *   the first symbol to encode (ie. the last symbol to decode)
 * - rans_encode_zero: initialize with 0
 *
 * rans_encode_init_with_prob allows to start from smaller state and
 * thus likely allows a better compression.
 *
 * rans_encode_zero allows even better compression. a downside is
 * that it requires the decoder know the exact size of the encoded bits.
 */
void rans_encode_init(struct rans_encode_state *st);
void rans_encode_init_zero(struct rans_encode_state *st);
void rans_encode_init_with_prob(struct rans_encode_state *st, rans_prob_t l_s);

void rans_encode_set_extra(struct rans_encode_state *st, rans_I extra);

struct bitbuf;
void rans_encode_sym(struct rans_encode_state *st, rans_sym_t sym,
                     rans_prob_t b_s, rans_prob_t l_s, struct bitbuf *bo);
void rans_encode_flush(struct rans_encode_state *st, struct bitbuf *bo);

#endif /* !defined(_RANS_ENCODE_H_) */
