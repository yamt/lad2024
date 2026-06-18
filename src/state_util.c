#include <string.h>

#include "defs.h"
#include "state.h"
#include "util.h"

void
upgrade_state(void)
{
        uint32_t version = state.version;
        CHECK(version >= 1 && version <= save_data_version);

        /* v1 -> v2 */
        if (version == 1) {
                struct save_data_v1 v1;
                memcpy(&v1, &state, sizeof(v1));

                memset(&state, 0, sizeof(state));
                memcpy(&state, &v1,
                       __builtin_offsetof(struct save_data, num_stats));
                state.num_stats = NUM_STATS;
                memcpy(state.clear_bitmap, v1.clear_bitmap,
                       sizeof(state.clear_bitmap));

                version = 2;
        }

        /* v2 -> v2; expands stats */
        uint32_t old_num_stats = state.num_stats;
        CHECK(old_num_stats <= NUM_STATS);
        if (old_num_stats != NUM_STATS) {
                uint32_t *old_clear_bitmap =
                        (uint32_t *)&state.stats[old_num_stats];
                memmove(state.clear_bitmap, old_clear_bitmap,
                        sizeof(state.clear_bitmap));
                memset(old_clear_bitmap, 0,
                       (uintptr_t)state.clear_bitmap -
                               (uintptr_t)old_clear_bitmap);
                state.num_stats = NUM_STATS;
        }

        CHECK(version == save_data_version);
}

void
validate_state(void)
{
        CHECK(state.cur_stage < nstages);
        CHECK(state.cleared_stages <= nstages);
        CHECK(state.num_stats == NUM_STATS);
        unsigned int n = 0;
        unsigned int i;
        for (i = 0; i < max_stages; i++) {
                if ((state.clear_bitmap[i / 32] & (1 << (i % 32))) != 0) {
                        n++;
                }
        }
        CHECK(state.cleared_stages == n);
}
