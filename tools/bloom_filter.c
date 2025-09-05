#include <math.h>

#include "bloom_filter.h"

/* https://en.wikipedia.org/wiki/Bloom_filter */

double
bloom_filter_opt_k(unsigned int m, unsigned int n)
{
        /* l(2) ~= .69314718055994530941 */
        return 0.69314718055994530941 * m / n;
}

double
bloom_filter_false_pos_prob(unsigned int m, unsigned int n, unsigned int k)
{
        return pow(1.0 - exp(-(double)k * n / m), k);
}
