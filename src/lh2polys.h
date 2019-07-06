#include "lfsr.h"

extern lfsr_poly_t poly_pairs[32];

int find_best_poly(uint32_t sample, uint32_t mask, uint32_t start = 0, uint32_t end = 32);