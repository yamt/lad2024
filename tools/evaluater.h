#include "node.h"

struct evaluation {
        unsigned int score;
};

void evaluate(const map_t map, const struct node_slist *solution,
              bool verbose, struct evaluation *ev);
