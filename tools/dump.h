#include "rule.h"

char objchr(uint8_t objidx);
char dirchr(enum diridx dir);
void dump_map(const map_t map);
void dump_map_raw(const map_t map);
void dump_map_c(const map_t map, const char *filename);