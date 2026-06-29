#include <stdint.h>

#define max_stages 1400
#define save_data_version 2

struct stage_clear_record {
        uint32_t value;
        uint32_t stage; /* stage number - 1 */
};

enum stat {
        STAT_TOTAL_TICK,
        STAT_GIVEUP_COUNT,
        STAT_UNDO_COUNT,
        STAT_MOVE_A_COUNT,
        STAT_AUTOMOVE_A_COUNT,
        STAT_MOVE_P_COUNT,
        STAT_AUTOMOVE_P_COUNT,
        STAT_PUSH_A_COUNT,
        STAT_PUSH_P_COUNT,
        STAT_GET_BOMB_COUNT,
        STAT_CLEAR_AGAIN, /* clear already-cleared stage again */
        STAT_TOTAL_CLEAR_TICK, /* the sum of stage_ticks for cleared stages */
        STAT_TOTAL_CLEAR_UNDO, /* the sum of stage_undo_count for cleared stages */
        STAT_TOTAL_CLEAR_MOVE, /* the sum of stage_move_count for cleared stages */

        NUM_STATS,
};

#define NRECORDS 10
#define CLEAR_BITMAP_NELEMS (((max_stages + 32 - 1) / 32))

struct save_data { /* v2 */
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t num_stats;
        struct stage_clear_record ticks_records[NRECORDS];
        struct stage_clear_record move_records[NRECORDS];
        struct stage_clear_record undo_records[NRECORDS];
        uint64_t stats[NUM_STATS]; /* XXX uint64_t is probably overkill? */
        uint32_t clear_bitmap[CLEAR_BITMAP_NELEMS];
};

struct save_data_v1 {
        uint32_t version;
        uint32_t cur_stage;
        uint32_t cleared_stages;
        uint32_t clear_bitmap[CLEAR_BITMAP_NELEMS];
};

extern struct save_data state;

#define STAT(x) (state.stats[STAT_##x])

void load_state(void);
void save_state(void);
void upgrade_state(void);
void validate_state(void);
