#include <stdio.h>

#include "dump.h"

char
objchr(uint8_t objidx)
{
        return "_WBXLDRUPA"[objidx];
}

void
dump_map(const map_t map)
{
        int x;
        int y;
        printf("  ");
        for (x = 0; x < width; x++) {
                printf("%2u", x);
        }
        printf("\n");
        for (y = 0; y < height; y++) {
                printf("%2u", y);
                for (x = 0; x < width; x++) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        int chr = objchr(objidx);
                        printf(" %c", chr);
                }
                printf("\n");
        }
}
