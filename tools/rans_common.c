#include "rans_common.h"

rans_prob_t
rans_b(const rans_prob_t ls[RANS_NSYMS], rans_sym_t sym)
{
        rans_prob_t b = 0;
        rans_sym_t i;
        for (i = 0; i < sym; i++) {
                b += ls[i];
        }
        return b;
}
