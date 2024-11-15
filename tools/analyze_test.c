#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyze.h"
#include "defs.h"
#include "dump.h"
#include "loader.h"
#include "maputil.h"

int
main(int argc, char **argv)
{
		/* all stages should be solvable */
        unsigned int i;
        for (i = 0; i < nstages; i++) {
           	    map_t map;
                struct map_info info;
                decode_stage(i, map, &info);
                if (tsumi(map)) {
                    printf("unexpected tsumi on stage %03u\n", i + 1);
                    dump_map(map);
                    exit(1);
                }
        }
        exit(0);
}
