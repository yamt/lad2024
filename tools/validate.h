#include <stdbool.h>

struct solution;
struct solver_param;

/*
 * validate/validate_slow are called after a refinement attempt
 * to check if the stage is still solvable.
 *
 * they return true if the refinement should be reverted.
 */
bool validate(const map_t map, const struct solution *solution, bool verbose,
              bool allow_removed_players);
bool validate_slow(const map_t map, struct solution *solution,
                   const struct solver_param *param, bool verbose,
                   bool allow_removed_players, struct solution *new_solution);
