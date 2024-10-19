#include <stdbool.h>

struct solution;
struct solver_param;

bool validate(const map_t map, const struct solution *solution, bool verbose,
              bool allow_removed_players);
bool validate_slow(const map_t map, const struct solution *solution,
                   const struct solver_param *param, bool verbose,
                   bool allow_removed_players);
