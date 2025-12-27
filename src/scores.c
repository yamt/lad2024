#include "scores.h"

static const struct score *scores[] = {
	&score1,
	&score2,
};

const struct score *
pick_score(uint32_t index)
{
	return scores[index % (sizeof(scores) / sizeof(scores[0]))];
}
