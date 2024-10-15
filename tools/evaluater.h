#include "node.h"

struct evaluation {
        unsigned int score;
};

void evaluate(const map_t map, const struct node_list *solution,
              struct evaluation *ev);
