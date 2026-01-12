/*
 * reference:
 *   Asymmetric numeral systems: entropy coding combining speed of
 *   Huﬀman coding with compression rate of arithmetic coding
 *   Jarek Duda
 */

#if !defined(_RANS_PARAM_H_)
#define _RANS_PARAM_H_

#include <stdint.h>

typedef uint8_t sym_t;
typedef uint16_t prob_t;
typedef uint32_t I;

#define B 256       /* b in the paper */
#define L (128 * M) /* l in the paper */
#define M 65536     /* m in the paper */

/*
 * I   = {L, ..., L * B - 1}
 * I_s = {L / M * l_s, ..., B * L / M * l_s - 1}
 */

#define I_SYM_MIN(p_sym) ((I)L / M * p_sym)
#define I_SYM_MAX(p_sym) ((I)B * L / M * p_sym - 1)

#define I_MIN ((I)L)
#define I_MAX ((I)L * B - 1)

#define NSYMS 256

#endif /* !defined(_RANS_PARAM_H_) */
