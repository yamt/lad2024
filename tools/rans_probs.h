#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        rans_prob_t ls[RANS_NSYMS]; /* l_s in the paper */
};

void count_syms(size_t counts[RANS_NSYMS], const void *input,
                size_t inputsize);
void rans_probs_init(struct rans_probs *ps, size_t ops[RANS_NSYMS]);

#define RANS_TABLE_MAX_NELEMS RANS_NSYMS
void rans_probs_table(const struct rans_probs *ps, rans_prob_t *out,
                      size_t *nelemp);
void rans_probs_table_with_trans(const struct rans_probs *ps, rans_prob_t *out,
                                 rans_sym_t *trans, size_t *nelemp);
