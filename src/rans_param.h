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

/*
 * a straightforward streaming rANS implementation
 *
 * optimized for size and simplicity.
 * ie. do not have extra tables for performance.
 *
 * an expected usage:
 * - the encoder runs on an x86-sized machine
 * - the decoder runs on a restricted environment, say, 64KB memory
 * - the decoder can trust input streams
 *
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#if !defined(_RANS_PARAM_H_)
#define _RANS_PARAM_H_

#include <stdint.h>

#include "rans_override.h"

#if !defined(RANS_SYM_TYPE)
#define RANS_SYM_TYPE uint8_t
#endif
#if !defined(RANS_PROB_TYPE)
#define RANS_PROB_TYPE uint16_t
#endif
#if !defined(RANS_I_TYPE)
#define RANS_I_TYPE uint32_t
#endif

typedef RANS_SYM_TYPE rans_sym_t;
typedef RANS_PROB_TYPE rans_prob_t;
typedef RANS_I_TYPE rans_I;

#if !defined(RANS_B_BITS)
#define RANS_B_BITS 8
#endif
#if !defined(RANS_B)
#define RANS_B (1 << RANS_B_BITS) /* b in the paper */
#endif
#if !defined(RANS_L)
#define RANS_L (256 * RANS_M) /* l in the paper */
#endif
#if !defined(RANS_M)
#define RANS_M 65536 /* m in the paper */
#endif

#define RANS_I_SYM_MIN(l_s) ((rans_I)RANS_L / RANS_M * l_s)
#define RANS_I_SYM_MAX(l_s) ((rans_I)RANS_B * (RANS_L / RANS_M) * l_s - 1)

#define RANS_I_MIN ((rans_I)RANS_L)
#define RANS_I_MAX ((rans_I)RANS_L * RANS_B - 1)

#define RANS_EXTRA_MAX (RANS_I_MAX - RANS_I_MIN)

#define RANS_NSYMS 256

#if !defined(RANS_ASSERT)
#include <assert.h>
#define RANS_ASSERT(a) assert(a)
#endif

#if RANS_B_BITS > 16
#error B > 65536 is not implemented
#endif
#if (1 << RANS_B_BITS) != RANS_B
#error inconsistent B and B_BITS
#endif
#if (RANS_L % RANS_M) != 0
#error L should be a multiple of M
#endif

/*
 * note: RANS_DECODE_BITS is just a hint for the user application.
 */
#if RANS_B_BITS == 8
#undef RANS_DECODE_BITS
#else
#define RANS_DECODE_BITS
#endif

#if !defined(ctassert)
#if __STDC_VERSION__ >= 201112L || __has_extension(c_static_assert)
#define ctassert(e) _Static_assert(e, #e)
#else
#define ctassert(e)
#endif
#endif

ctassert((rans_prob_t)(RANS_M - 1) == RANS_M - 1);
ctassert((rans_I)RANS_I_MAX == RANS_I_MAX);
ctassert(RANS_I_MAX / RANS_L == RANS_B - 1);
ctassert(RANS_I_MAX / RANS_B == RANS_L - 1);

#endif /* !defined(_RANS_PARAM_H_) */
