#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "state.h"

struct save_data state;

static void
print_records(const struct stage_clear_record *ts)
{
        unsigned int i;
        for (i = 0; i < NRECORDS; i++) {
                const struct stage_clear_record *t = &ts[i];
                if (t->value == 0) {
                        assert(t->stage == 0);
                        printf("[%2d] ----\n", i + 1);
                        continue;
                }
                printf("[%2d] stage %d value %d\n", i + 1, t->stage + 1,
                       t->value);
        }
}

static void
dump_state(void)
{
        printf("version %" PRIu32 "\n", state.version);
        printf("cur_stage %" PRIu32 "\n", state.cur_stage + 1);
        printf("cleared_stages %" PRIu32 "\n", state.cleared_stages);
        printf("num_stats %" PRIu32 "\n", state.num_stats);

#define PRINT_RECORDS(x)                                                      \
        printf("=== %s top-n ===\n", #x);                                     \
        print_records(state.x##_records)

        PRINT_RECORDS(ticks);
        PRINT_RECORDS(undo);
        PRINT_RECORDS(move);

        printf("=== stats ===\n");
#define PRINT_STAT(x)                                                         \
        printf("%s %d\n", #x, (unsigned int)state.stats[STAT_##x])
        PRINT_STAT(TOTAL_TICK);
        PRINT_STAT(GIVEUP_COUNT);
        PRINT_STAT(UNDO_COUNT);
        PRINT_STAT(MOVE_A_COUNT);
        PRINT_STAT(AUTOMOVE_A_COUNT);
        PRINT_STAT(MOVE_P_COUNT);
        PRINT_STAT(AUTOMOVE_P_COUNT);
        PRINT_STAT(PUSH_A_COUNT);
        PRINT_STAT(PUSH_P_COUNT);
        PRINT_STAT(GET_BOMB_COUNT);
        PRINT_STAT(CLEAR_AGAIN);

        printf("=== clear_bitmap ===\n");

        unsigned int start = 0;
        bool prev = false;

        unsigned int i;
        for (i = 0; i < max_stages; i++) {
                bool cleared =
                        (state.clear_bitmap[i / 32] & (1 << (i % 32))) != 0;
                if (i == 0) {
                        prev = cleared;
                }
                if (prev != cleared) {
                        printf("stage %u-%u %d\n", start + 1, i, prev);
                        start = i;
                        prev = cleared;
                }
        }
        if (start + 1 != i) {
                printf("stage %u-%u %d\n", start + 1, i, prev);
        }
}

int
main(int argc, char **argv)
{
        const char *filename = argv[1];
        if (argc != 2) {
                fprintf(stderr, "arg error\n");
                exit(2);
        }
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
                fprintf(stderr, "open %s failed: %s\n", filename,
                        strerror(errno));
                exit(1);
        }
        memset(&state, 0, sizeof(state));
        ssize_t ssz = read(fd, &state, sizeof(state));
        if (ssz == -1) {
                fprintf(stderr, "read failed: %s\n", strerror(errno));
                exit(1);
        }

        printf("file version %" PRIu32 "\n", state.version);
        upgrade_state();
        validate_state();
        dump_state();
}
