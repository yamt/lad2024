#if !defined(NDEBUG)
#if defined(__wasm__)
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
#include <assert.h>
#define ASSERT(cond) assert(cond)
#endif
#else
#define ASSERT(cond)                                                          \
        do {                                                                  \
                if (!(cond)) {                                                \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#endif
