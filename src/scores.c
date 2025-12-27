#include "scores.h"

static const struct score *scores[] = {
	&score1,
	&score2,
	&score3,
	&score4,
};

const struct score *
pick_score(uint32_t idx)
{
	return scores[idx % (sizeof(scores) / sizeof(scores[0]))];
}
