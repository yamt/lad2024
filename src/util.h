#if defined(NDEBUG)
#define CHECK(cond)                                                           \
        do {                                                                  \
                if (!(cond)) {                                                \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#else /* defined(NDEBUG) */
#define CHECK(cond)                                                           \
        do {                                                                  \
                if (!(cond)) {                                                \
                        tracef("assertion (%s) failed at %s:%d", #cond,       \
                               __FILE__, __LINE__);                           \
                        __builtin_trap();                                     \
                }                                                             \
        } while (0)
#endif /* defined(NDEBUG) */

#if defined(NDEBUG)
#define ASSERT(cond)
#else /* defined(NDEBUG) */
#if defined(__wasm__)
#include "wasm4.h"
#define ASSERT(cond) CHECK(cond)
#else /* defined(__wasm__) */
#include <assert.h>
#define ASSERT(cond) assert(cond)
#endif /* defined(__wasm__) */
#endif /* defined(NDEBUG) */
