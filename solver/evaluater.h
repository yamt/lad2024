#include "node.h"

struct evaluation {
        unsigned int score;
};

void evaluate(const struct node *root, const struct node_list *solution,
              struct evaluation *ev);
