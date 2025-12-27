#include <stdint.h>

struct score;

extern const struct score score1;
extern const struct score score2;
extern const struct score score3;

const struct score *pick_score(uint32_t idx);
