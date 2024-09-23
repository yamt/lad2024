#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
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
        for (x = 0; x < map_width; x++) {
                printf("%2u", x);
        }
        printf("\n");
        for (y = 0; y < map_height; y++) {
                printf("%2u", y);
                for (x = 0; x < map_width; x++) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        int chr = objchr(objidx);
                        printf(" %c", chr);
                }
                printf("\n");
        }
}

void
dump_map_c(const map_t map, const char *filename)
{
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
                fprintf(stderr, "failed to open %s\n", filename);
                exit(1);
        }
        int x;
        int y;
        fprintf(fp, "\t{\n");
        fprintf(fp, "\t\t.data = (const uint8_t[]){\n");
        for (y = 0; y < map_height; y++) {
                const char *sep = "\t\t\t";
                int line_size = map_width;
                for (x = map_width - 1; x >= 0; x--) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        if (objidx != _) {
                                break;
                        }
                        line_size = x;
                }
                if (line_size == 0) {
                        break;
                }
                for (x = 0; x < line_size; x++) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        int chr = objchr(objidx);
                        fprintf(fp, "%s%c,", sep, chr);
                        sep = "";
                }
                fprintf(fp, " END,\n");
        }
        fprintf(fp, "\t\t\tEND, END,\n");
        fprintf(fp, "\t\t},\n");
        fprintf(fp, "\t},\n");
        fclose(fp);
}
