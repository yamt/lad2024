#include <sys/stat.h>

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * unpack 1 bit.
 *
 * bp: a buffer
 * cl: (the number of valid bits in bp) + 1
 */
void
unpack_dat(uint16_t *bp, uint16_t **di, uint8_t *ch, uint8_t *cl, uint16_t *dx,
           uint16_t *ax)
{
        // printf("%s ch %x cl %x\n", __func__, (int)*ch, (int)*cl);
        if ((--(*cl)) == 0) {
                goto u1;
        }
u0: {
        /*
         * shl bp,1
         * rcl ax,1
         */
        *ax <<= 1;
        if ((*bp & 0x8000) != 0) {
                *ax |= 1;
        }
        *bp <<= 1;
}
        *ax &= *ch;
        // printf("%s      => %x\n", __func__, (int)*ax);
        return;
u1:
        *bp = **di; /* little endian is assumed */
        (*di)++;
        // printf("%s %04x =>\n", __func__, (int)*bp);
        *cl = 16;
        (*dx)--;
        if (*dx != 0) {
                goto u0;
        }
        *ch = 6;
        goto u0;
}

#define width 40
#define height 25

static const int16_t vects[] = {-1, width, 1, -width, -1};

uint8_t maptmp[width * height * 3];

void
dat2map(uint16_t *bp, uint8_t **bx, uint16_t **di, uint8_t *ch, uint8_t *cl,
        uint16_t *dx, int16_t ax)
{
        uint8_t *p = *bx;
        p += ax;
        if (p[width * height] == 0) {
                goto d2m_end;
        }
        p[width * height] = 0;
        /* extract 3 bits */
        uint16_t result = 0;
        unpack_dat(bp, di, ch, cl, dx, &result);
        unpack_dat(bp, di, ch, cl, dx, &result);
        unpack_dat(bp, di, ch, cl, dx, &result);
        result++;
        result &= 7;
        // printf("unpacked ax %x\n", result);
        *p = (uint8_t)result;
        if (result == 1) {
                goto d2m_end;
        }
        const int16_t *si = vects;
        do {
                int16_t ax = *si++;
                dat2map(bp, &p, di, ch, cl, dx, ax);
        } while (si != vects + 4);
d2m_end:;
}

void
manu_map()
{
        uint8_t *p;
        for (p = maptmp; p < maptmp + width * height; p++) {
                if (*p == 0xff) {
                        int cx = 4;
                        do {
                                const int16_t *v = (vects - 1) + cx;
                                if (p[v[0]] == 1 && p[v[1]] == 1 &&
                                    p[v[0] + v[1]] != 1) {
                                        *p = 1;
                                }
                        } while ((--cx) != 0);
                }
        }
}

void
load_map(const uint8_t *p)
{
        /* p == si */

        memset(maptmp, 0xff, width * height * 2);

        /* p[0] size */
        uint16_t dat_len = p[0];
        uint16_t jos_y = p[3];
        uint16_t jos_x = p[4];
        uint8_t *bx = jos_y * width + jos_x + maptmp;
        uint16_t dx = (dat_len - (1 + 4 - 2)) >> 1;
        uint8_t ch = 0x07;
        uint8_t cl = 0x01;
        uint16_t *di = (uint16_t *)&p[5]; /* alignment */
        int16_t ax = 0;
        uint16_t bp = 0;

        dat2map(&bp, &bx, &di, &ch, &cl, &dx, ax);
        manu_map();
}

const unsigned char stg[] = {0x11, 0x07, 0x0c, 0x07, 0x0e, 0xf1,
                             0xfb, 0xe3, 0xc0, 0x38, 0x8e, 0x7f,
                             0x1c, 0x1c, 0xff, 0xb8, 0x24, 0};

/*
 * fbf1 == 1111 1011 1111 0001
 * 111 110 111 111 000 1..
 *
 * 0  floor
 * 1  wall
 * 2  box
 * 3  bomb
 * 4  light (left)
 * 5  light (down)
 * 6  light (right)
 * 7  light (up)
 */

void
dump(const uint8_t *p)
{
        int x;
        int y;
        for (y = 0; y < height; y++) {
                const char *sep = "";
                for (x = 0; x < height; x++) {
                        printf("%s%02x", sep, p[y * width + x]);
                        sep = " ";
                }
                printf("\n");
        }
}

void
convert_to_c(int stage, const uint8_t *map, uint8_t hks_x, uint8_t hks_y,
             uint8_t jos_x, uint8_t jos_y)
{
        int xmax = 0;
        int ymax = 0;
        int xmin = width;
        int ymin = height;
        int x;
        int y;
        for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                        if (map[y * width + x] != 0xff) {
                                if (xmin > x) {
                                        xmin = x;
                                }
                                if (ymin > y) {
                                        ymin = y;
                                }
                                if (xmax < x) {
                                        xmax = x;
                                }
                                if (ymax < y) {
                                        ymax = y;
                                }
                        }
                }
        }
        const int newwidth = 20;
        const int newheight = 20;
        if (xmax - xmin >= newwidth || ymax - ymin >= newheight) {
                fprintf(stderr, "stage too large\n");
                abort();
        }
        char ch[] = {
                [0] = '_', [1] = 'W', [2] = 'B', [3] = 'X', [4] = 'L',
                [5] = 'D', [6] = 'R', [7] = 'U', [8] = 'P', [9] = 'A',
        };
        printf("    {\n");
        printf("        .data = (const uint8_t[]){\n");
        for (y = 0; y < ymax - ymin + 1; y++) {
                printf("            ");
                int linexmax = 0;
                for (x = 0; x < newwidth; x++) {
                        int idx = map[(y + ymin) * width + (x + xmin)];
                        if (idx != 0 && idx != 0xff) {
                                linexmax = x;
                        }
                }
                for (x = 0; x < linexmax + 1; x++) {
                        int idx;
                        if (hks_y == y + ymin && hks_x == x + xmin) {
                                idx = 8;
                        } else if (jos_y == y + ymin && jos_x == x + xmin) {
                                idx = 9;
                        } else {
                                idx = map[(y + ymin) * width + (x + xmin)];
                                if (idx == 0xff) {
                                        idx = 0;
                                }
                        }
                        printf("%c, ", ch[idx]);
                }
                printf("END,\n");
        }
        printf("            END,\n");
        printf("        },\n");
        printf("        .message =\n");
        printf("        \"Converted from\\n\"\n");
        printf("        \"stage %04u of\\n\"\n", stage);
        printf("        \"the 1994 version.\\n\"\n");
        printf("    },\n");
}

int
main(int argc, char **argv)
{
        const char *filename = argv[1];
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
                exit(1);
        }
        struct stat st;
        if (fstat(fd, &st) == -1) {
                exit(1);
        }
        size_t sz = st.st_size;
        uint8_t *buf = malloc(sz);
        if (buf == NULL) {
                exit(1);
        }
        if (read(fd, buf, sz) != sz) {
                exit(1);
        }
        close(fd);

        int stage = 1;
        uint8_t *p = buf;
        while (p < buf + sz) {
                // printf("== stage %04d\n", stage);
                // printf("hks_y %u hks_x %u jos_y %u jos_x %u\n", p[1], p[2],
                // p[3], p[4]);
                load_map(p);
                // dump(maptmp);
                convert_to_c(stage, maptmp, p[2], p[1], p[4], p[3]);
                p += p[0];
                stage++;
        }
}
