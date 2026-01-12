#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        prob_t ls[NSYMS]; /* l_s in the paper */
};

void count_syms(size_t counts[NSYMS], const void *input, size_t inputsize);
void rans_probs_init(struct rans_probs *ps, size_t ops[NSYMS]);

#define RANS_TABLE_MAX_NELEMS NSYMS
void rans_probs_table(const struct rans_probs *ps, prob_t *out,
                      size_t *nelemp);
