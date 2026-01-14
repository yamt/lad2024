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

typedef uint8_t rans_sym_t;
typedef uint8_t rans_prob_t;
typedef uint32_t rans_I;

#define RANS_B_BITS 8
/* #define RANS_DECODE_BITS */
#define RANS_B (1 << RANS_B_BITS) /* b in the paper */
#define RANS_L (1 * RANS_M)     /* l in the paper */
#define RANS_M 256              /* m in the paper */

#define RANS_I_SYM_MIN(l_s) ((rans_I)RANS_L / RANS_M * l_s)
#define RANS_I_SYM_MAX(l_s) ((rans_I)RANS_B * RANS_L / RANS_M * l_s - 1)

#define RANS_I_MIN ((rans_I)RANS_L)
#define RANS_I_MAX ((rans_I)RANS_L * RANS_B - 1)

#define RANS_EXTRA_MAX (RANS_I_MAX - RANS_I_MIN)

#define RANS_NSYMS 256

#include <assert.h>
#define RANS_ASSERT(a) assert(a)

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
