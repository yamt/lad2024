#if !defined(NDEBUG)
#include "wasm4.h"
#define ASSERT(cond)                                                          \
        do {                                                                  \
                if (!(cond)) {                                                \
                        tracef("assertion (%s) failed at %s:%d", #cond,       \
                               __FILE__, __LINE__);                           \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#else
#define ASSERT(cond)                                                          \
        do {                                                                  \
                if (!(cond)) {                                                \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#endif
