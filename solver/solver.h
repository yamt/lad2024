#include "node.h"

#define SOLVE_SOLVED 0x01
#define SOLVE_IMPOSSIBLE 0x02
#define SOLVE_GIVENUP 0x03

struct node;
unsigned int solve(struct node *root, unsigned int max_iterations,
                   struct node_list *solution);
void solve_cleanup(void);
