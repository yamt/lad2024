#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lz.h"

ssize_t
readall(int fd, void *buf, size_t sz)
{
        ssize_t rsz;
        size_t offset = 0;
        while (offset < sz) {
                rsz = read(fd, (uint8_t *)buf + offset, sz - offset);
                if (rsz == -1) {
                        if (errno == EINTR) {
                                continue;
                        }
                        break;
                }
                if (rsz == 0) {
                        break;
                }
                offset += rsz;
        }
        if (offset > 0) {
                return offset;
        }
        return rsz;
}

static void
out_literal(void *ctx, uint8_t ch)
{
        printf("%s: ch 0x%02x\n", __func__, ch);
}

static void
out_match(void *ctx, woff_t dist, woff_t len)
{
        printf("%s: dist %u len %u\n", __func__, dist, len);
}

int
main(void)
{
        struct lz_encode_state s0;
        struct lz_encode_state *s = &s0;
        lz_encode_init(s);

        s->out_literal = out_literal;
        s->out_match = out_match;
        char buf[100];
        while (1) {
                ssize_t rsz = readall(STDIN_FILENO, buf, sizeof(buf));
                if (rsz == -1) {
                        fprintf(stderr, "read error: %s\n", strerror(errno));
                        exit(1);
                }
                if (rsz == 0) {
                        lz_encode_flush(s);
                        break;
                }
                lz_encode(s, buf, rsz);
        }
}
