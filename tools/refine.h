#include <stdbool.h>

#include "rule.h"

struct solution;
struct solver_param;

bool refine(map_t map, const struct solution *solution);
bool try_refine(map_t map, struct solution *solution,
                const struct solver_param *param);
