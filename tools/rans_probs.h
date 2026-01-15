#if !defined(_RANS_PROBS_H_)
#define _RANS_PROBS_H_

#include <stddef.h>

#include "rans_param.h"

struct rans_probs {
        rans_prob_t ls[RANS_NSYMS]; /* l_s in the paper */
};

void rans_probs_init(struct rans_probs *ps, const size_t ops[RANS_NSYMS]);

#define RANS_TABLE_MAX_NELEMS RANS_NSYMS
void rans_probs_table(const struct rans_probs *ps, rans_prob_t *out,
                      size_t *nelemp);
void rans_probs_table_with_trans(const struct rans_probs *ps, rans_prob_t *out,
                                 rans_sym_t *trans, size_t *nelemp);

/* the following functions are not really specific to rANS. */
void count_syms(size_t counts[RANS_NSYMS], const void *input,
                size_t inputsize);
size_t calc_sum(const size_t ps[RANS_NSYMS]);
double calc_bits(const size_t dist[RANS_NSYMS], size_t dist_sum,
                 const size_t real_dist[RANS_NSYMS]);
#endif /* defined(_RANS_PROBS_H_) */
