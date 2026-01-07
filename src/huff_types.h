#if !defined(_LAD_HUFF_TYPES_H_)
#define _LAD_HUFF_TYPES_H_

#include <stdint.h>

#if !defined(HUFF_NSYMS)
#define HUFF_NSYMS 256
#endif

#if HUFF_NSYMS <= 256
typedef uint8_t huff_sym_t;
#elif HUFF_NSYMS <= 65536
typedef uint16_t huff_sym_t;
#endif

#endif /* !defined(_LAD_HUFF_TYPES_H_) */
