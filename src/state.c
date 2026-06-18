#include <string.h>

#include "defs.h"
#include "state.h"
#include "util.h"

#include "wasm4.h"

struct save_data state;

void
load_state(void)
{
        /*
         * assumption: older versions are smaller.
         * NUM_STATS and max_stages monotonically increase.
         */
        memset(&state, 0, sizeof(state));
        diskr(&state, sizeof(state));
        if (state.version == 0) {
                /*
                 * initial state. (all zero)
                 *
                 * or maybe a save data from a version older than
                 * the first public release. (20240831)
                 * that version didn't have version field. we don't
                 * bother to handle that version anymore.
                 */
                unsigned int i;
                for (i = 0; i < sizeof(state); i++) {
                        CHECK(((const uint8_t *)&state)[i] == 0);
                }
                state.version = 1;
        }
        upgrade_state();
        validate_state();
}

void
save_state(void)
{
        /*
         * although this cart currently has nothing for netplay,
         * wasm-4 doesn't have a way for a cart to prevent users from
         * starting a netplay.
         *
         * if netplay is enabled, only perform diskw for the first player,
         * which is the host of the netplay.
         * in case of netplay, other players merely have a copy of the in-core
         * disk buffer from the first player. do not risk overwriting the
         * local storage with it.
         *
         * as this cart loads the data from the disk buffer only on start(),
         * it's ok to leave the disk buffer contents stale here.
         *
         * this can be even considered as a security measure as a bad person
         * can attempt to overwrite your save data by sending a netplay url.
         *
         * cf. https://github.com/aduros/wasm4/issues/837
         */
        if ((*NETPLAY & 3) != 0) {
                return;
        }
        state.version = save_data_version;
        diskw(&state, sizeof(state));
}
