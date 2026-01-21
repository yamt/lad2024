/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
