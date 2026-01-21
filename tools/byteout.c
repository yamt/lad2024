/*-
 * Copyright (c)2026 YAMAMOTO Takashi,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
