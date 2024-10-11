#include <stdbool.h>
#include <stddef.h>

#include "node.h"

#define SOLVE_SOLVED 0x01
#define SOLVE_IMPOSSIBLE 0x02
#define SOLVE_GIVENUP 0x03

/* proven to be solvable, but returning no solution */
#define SOLVE_SOLVABLE 0x04

struct solution {
        struct node_list moves;
        unsigned int nmoves;
        unsigned int iterations;
        enum giveup_reason {
                GIVEUP_MEMORY,
                GIVEUP_ITERATIONS,
        } giveup_reason;
};

struct solver_param {
        size_t limit;
        unsigned int max_iterations;
};

extern struct solver_param solver_default_param;

struct node;
unsigned int solve(struct node *root, const struct solver_param *param,
                   bool verbose, struct solution *solution);
void solve_cleanup(void);
