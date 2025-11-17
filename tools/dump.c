#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "dump.h"
#include "maputil.h"

void
dump_map_raw(const map_t map)
{
        struct size size;
        measure_size(map, &size);
        int x;
        int y;
        printf("  ");
        for (x = size.xmin; x <= size.xmax; x++) {
                printf("%2u", x);
        }
        printf("\n");
        for (y = size.ymin; y <= size.ymax; y++) {
                printf("%2u", y);
                for (x = size.xmin; x <= size.xmax; x++) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        printf("%2x", objidx);
                }
                printf("\n");
        }
}

char
objchr(uint8_t objidx)
{
        return "_WBXLDRUPA"[objidx];
}

char
dirchr(enum diridx dir)
{
        return "LDRU"[dir];
}

void
dump_map(const map_t map)
{
        struct size size;
        measure_size(map, &size);
        int x;
        int y;
        printf("  ");
        for (x = size.xmin; x <= size.xmax; x++) {
                printf("%2u", x);
        }
        printf("\n");
        for (y = size.ymin; y <= size.ymax; y++) {
                printf("%2u", y);
                for (x = size.xmin; x <= size.xmax; x++) {
                        loc_t loc = genloc(x, y);
                        uint8_t objidx = map[loc];
                        int chr = objchr(objidx);
                        printf(" %c", chr);
                }
                printf("\n");
        }
}

void
dump_map_c_to(const map_t map, FILE *fp)
{
        int x;
        int y;
        fprintf(fp, "    {\n");
        fprintf(fp, "        .data = (const uint8_t[]){\n");
        for (y = 0; y < map_height; y++) {
                const char *sep = "            ";
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
        fprintf(fp, "            END, END,\n");
        fprintf(fp, "        },\n");
        fprintf(fp, "    },\n");
}

void
dump_map_c(const map_t map, const char *filename)
{
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
                fprintf(stderr, "failed to open %s\n", filename);
                exit(1);
        }
        dump_map_c_to(map, fp);
        fclose(fp);
}

void
dump_map_c_fmt(const map_t map, const char *filename_fmt, ...)
{
        char filename[PATH_MAX];
        va_list ap;
        va_start(ap, filename_fmt);
        vsnprintf(filename, sizeof(filename), filename_fmt, ap);
        va_end(ap);
        dump_map_c(map, filename);
}
