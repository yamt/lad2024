#include <stdbool.h>

#include "rule.h"

struct solution;
struct solver_param;

bool refine(map_t map, bool eager, struct solution *solution,
            const struct solver_param *param);
bool try_refine(map_t map, struct solution *solution,
                const struct solver_param *param);
