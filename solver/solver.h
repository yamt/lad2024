#include <stdbool.h>
#include <stddef.h>

#include "node.h"

#define SOLVE_SOLVED 0x01
#define SOLVE_IMPOSSIBLE 0x02
#define SOLVE_GIVENUP 0x03

/* proven to be solvable, but returning no solution */
#define SOLVE_SOLVABLE 0x04

struct node;
unsigned int solve(struct node *root, size_t limit, bool verbose,
                   struct node_list *solution);
void solve_cleanup(void);
