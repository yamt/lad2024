#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "loader.h"
#include "stages.h"

size_t
stage_data_size(const uint8_t *p)
{
        const uint8_t *sp = p;
        uint8_t ch;
        while (true) {
                ch = *p++;
                if (ch == END) {
                        ch = *p++;
                        if (ch == END) {
                                return p - sp;
                        }
                }
        }
}

int
main(int argc, char **argv)
{
        if (argc != 2 && argc != 3) {
                exit(2);
        }
        int stage_number = atoi(argv[1]);
        int stage_number_max;
        if (stage_number < 0 || stage_number > nstages) {
                exit(2);
        }
        if (argc == 3) {
                stage_number_max = atoi(argv[2]);
                if (stage_number_max < 0 || stage_number > nstages) {
                        exit(2);
                }
        } else {
                stage_number_max = stage_number;
        }
        if (stage_number == 0) {
                stage_number = 1;
        }
        if (stage_number_max == 0) {
                stage_number_max = nstages;
        }

        unsigned int i;
        for (i = stage_number - 1; i < stage_number_max; i++) {
            const uint8_t *data = stages[i].data;
            size_t size = stage_data_size(data);
            fwrite(data, size, 1, stdout);
        }
}
