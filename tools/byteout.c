#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "byteout.h"

static void
extend(struct byteout *bo)
{
        size_t newsize = bo->allocated * 2;
        if (newsize < 16) {
                newsize = 16;
        }
        void *p = realloc(bo->p, newsize);
        if (p == NULL) {
                fprintf(stderr, "byteout realloc failed\n");
                exit(1);
        }
        bo->p = p;
        bo->allocated = newsize;
}

void
byteout_init(struct byteout *bo)
{
        bo->p = NULL;
        bo->actual = 0;
        bo->allocated = 0;
}

void
byteout_write(struct byteout *bo, uint8_t byte)
{
        if (bo->actual >= bo->allocated) {
                extend(bo);
        }
        bo->p[bo->actual++] = byte;
}

void
byteout_clear(struct byteout *bo)
{
        free(bo->p);
}

void
rev_byteout_write(struct byteout *bo, uint8_t byte)
{
        if (bo->actual >= bo->allocated) {
                extend(bo);
                memmove(bo->p + bo->allocated - bo->actual, bo->p, bo->actual);
        }
        bo->p[bo->allocated - (++bo->actual)] = byte;
}

void *
rev_byteout_ptr(const struct byteout *bo)
{
        return &bo->p[bo->allocated - bo->actual];
}
