#if !defined(_RANS_DECODE_H_)
#define _RANS_DECODE_H_

#include <stdbool.h>

#include "rans_param.h"

struct rans_decode_state {
        rans_I x;
};

#if defined(RANS_DECODE_BITS)
struct bitin;
typedef struct bitin *rans_decode_input_type;
#else
typedef const uint8_t **rans_decode_input_type;
#endif

void rans_decode_init(struct rans_decode_state *st);
rans_sym_t rans_decode_sym(struct rans_decode_state *st,
                           const rans_prob_t ls[RANS_NSYMS],
                           rans_decode_input_type inpp);
rans_I rans_decode_get_extra(struct rans_decode_state *st,
                             rans_decode_input_type inpp);

#endif /* defined(_RANS_DECODE_H_) */
