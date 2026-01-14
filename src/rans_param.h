/*
 * a straightforward rANS implementation
 *
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#if !defined(_RANS_PARAM_H_)
#define _RANS_PARAM_H_

#include <stdint.h>

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
/* #define RANS_DECODE_BITS */
#if !defined(RANS_B)
#define RANS_B (1 << RANS_B_BITS) /* b in the paper */
#endif
#if !defined(RANS_L)
#define RANS_L (128 * RANS_M)     /* l in the paper */
#endif
#if !defined(RANS_M)
#define RANS_M 65536              /* m in the paper */
#endif

#define RANS_I_SYM_MIN(l_s) ((rans_I)RANS_L / RANS_M * l_s)
#define RANS_I_SYM_MAX(l_s) ((rans_I)RANS_B * RANS_L / RANS_M * l_s - 1)

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

#endif /* !defined(_RANS_PARAM_H_) */
