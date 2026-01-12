/*
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#if !defined(_RANS_PARAM_H_)
#define _RANS_PARAM_H_

#include <stdint.h>

typedef uint8_t rans_sym_t;
typedef uint16_t rans_prob_t;
typedef uint32_t rans_I;

#define RANS_B 256            /* b in the paper */
#define RANS_L (128 * RANS_M) /* l in the paper */
#define RANS_M 65536          /* m in the paper */

/*
 * I   = {L, ..., L * B - 1}
 * I_s = {L / M * l_s, ..., B * L / M * l_s - 1}
 */

#define RANS_I_SYM_MIN(p_sym) ((rans_I)RANS_L / RANS_M * p_sym)
#define RANS_I_SYM_MAX(p_sym) ((rans_I)RANS_B * RANS_L / RANS_M * p_sym - 1)

#define RANS_I_MIN ((rans_I)RANS_L)
#define RANS_I_MAX ((rans_I)RANS_L * RANS_B - 1)

#define RANS_NSYMS 256

#endif /* !defined(_RANS_PARAM_H_) */
