#include "rans_common.h"

prob_t
rans_b(const prob_t ls[NSYMS], sym_t sym)
{
        prob_t b = 0;
        sym_t i;
        for (i = 0; i < sym; i++) {
                b += ls[i];
        }
        return b;
}
