#include <stdbool.h>
#include <stddef.h>

#include "node.h"

#define SOLVE_SOLVED 0x01
#define SOLVE_IMPOSSIBLE 0x02
#define SOLVE_GIVENUP 0x03

/* proven to be solvable, but only returning a partial solution */
#define SOLVE_SOLVABLE 0x04

struct solver_stat {
        unsigned int iterations;
        unsigned int nodes;
};

struct solution {
        bool detached;
        struct node_slist moves;
        unsigned int nmoves;
        enum giveup_reason {
                GIVEUP_MEMORY,
                GIVEUP_ITERATIONS,
        } giveup_reason;
        struct solver_stat stat;
};

struct solver_param {
        size_t limit;
        unsigned int max_iterations;
};

extern struct solver_param solver_default_param;

struct node;
unsigned int solve(const map_t map, const struct solver_param *param,
                   bool verbose, struct solution *solution);
void solve_cleanup(void);

void detach_solution(struct solution *solution);
void clear_solution(struct solution *solution);
