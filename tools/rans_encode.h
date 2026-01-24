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

#if !defined(_RANS_ENCODE_H_)
#define _RANS_ENCODE_H_

#include "rans_param.h"

struct rans_encode_state {
        rans_I x;
};

/*
 * a few alternatives for encoder state initialization
 *
 * - rans_encode_init: initialize with I_MIN
 * - rans_encode_init_with_prob: initialize with the probability of
 *   the first symbol to encode (ie. the last symbol to decode)
 * - rans_encode_init_zero: initialize with 0
 *
 * rans_encode_init_with_prob allows to start from smaller state and
 * thus likely allows a better compression.
 *
 * rans_encode_zero allows even better compression. a downside is
 * that it requires the decoder know the exact size of the encoded bits.
 *
 * note: "better compression" here usually means up to a few bytes
 * differences. if your data is large enough, maybe the difference is
 * negligible. in that case, just use rans_encode_init as it's simplest
 * to use.
 */
void rans_encode_init(struct rans_encode_state *st);
void rans_encode_init_zero(struct rans_encode_state *st);
void rans_encode_init_with_prob(struct rans_encode_state *st, rans_prob_t l_s);

/*
 * rans_encode_set_extra: add extra value to the state so that the
 * decoder later can extract it with rans_decode_get_extra.
 * this mechanism can be used to compensate the cost to store the final
 * state of the encoding.
 * rans_encode_set_extra should be called before encoding any symbols.
 * otoh, rans_decode_get_extra should be called after decodinf all symbols.
 *
 * note: the current implementation is incompatible with
 * rans_encode_init_zero/rans_encode_init_with_prob.
 */
void rans_encode_set_extra(struct rans_encode_state *st, rans_I extra);

struct bitbuf;
void rans_encode_sym(struct rans_encode_state *st, rans_sym_t sym,
                     rans_prob_t b_s, rans_prob_t l_s, struct bitbuf *bo);
void rans_encode_flush(struct rans_encode_state *st, struct bitbuf *bo);

#endif /* !defined(_RANS_ENCODE_H_) */
