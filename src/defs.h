#if !defined(_LAD_DEFS_H)
#define _LAD_DEFS_H

#include <stdint.h>

enum block {
        _ = 0, /* floor */
        W,     /* wall */
        B,     /* box */
        X,     /* bomb */
        L,     /* light left */
        D,     /* light down */
        R,     /* light right */
        U,     /* light up */
        P,     /* person */
        A,     /* robot */
        END,
};

extern const unsigned int nstages;

#endif /* !defined(_LAD_DEFS_H) */
