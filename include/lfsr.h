#pragma once

#include <tuple>
#include "assert.h"
#include <vector>

typedef uint32_t lfsr_poly_t;
typedef uint32_t lfsr_state_t;

struct lfsr_t {
    lfsr_poly_t poly;
    lfsr_state_t state;

    lfsr_t(lfsr_poly_t poly, lfsr_state_t state = 1);
};

#ifdef HAS_BUILTIN_POPCOUNT
static inline uint8_t popcnt(uint32_t x) {
    return __builtin_popcount(x);
}
#else
template <typename T>
static inline uint8_t popcnt(T x)
{
    int c;
    for (c = 0; x != 0; x >>= 1u)
        if (x & 1u)
            c++;
    return c;
}
#endif

static inline uint32_t reverse32(uint32_t v) {
    uint32_t rtn = 0;
    for(int i = 0;i < 32;i++) {
        rtn = rtn << 1;
        rtn |= v & 1;
        v = v >> 1;
    }
    return rtn;
}


uint32_t lfsr_period(lfsr_poly_t p);
uint32_t lfsr_find(lfsr_poly_t p, lfsr_state_t start, lfsr_state_t end);
uint32_t lfsr_find_with_mask(lfsr_poly_t p, lfsr_state_t start, lfsr_state_t state, uint32_t mask);
uint32_t lfsr_error(lfsr_poly_t p, lfsr_state_t state, uint32_t mask);

void print_binary(uint32_t v, const std::string& postfix = "\n");
void lsfr_print(lfsr_state_t start, lfsr_poly_t p);
lfsr_poly_t lsfr_mirror_poly(lfsr_poly_t poly);
std::string lsfr_iterate_str(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt = 1);
std::vector<bool> lsfr_iterate_vec(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt = 1);
lfsr_state_t lsfr_iterate(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt = 1);
lfsr_t lsfr_iterate(lfsr_t lsfr, uint32_t cnt = 1);
int lfsr_order(lfsr_poly_t v);

struct lfsr_lookup_t;

lfsr_lookup_t* lfsr_lookup_ctor(lfsr_poly_t p);
uint32_t lfsr_lookup_query(lfsr_lookup_t* lookup, uint32_t q);
