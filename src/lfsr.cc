#include <bitset>
#include "lfsr.h"
#include "stdlib.h"

lfsr_state_t lsfr_iterate(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt) {
    for(int i = 0;i < cnt;i++) {
        uint16_t b = popcnt(state & poly) & 1u;
        state = (state << 1u) | b;
    }
    return state;
}
std::string lsfr_iterate_str(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt) {
    std::string rtn = std::bitset<32>(state).to_string();
    for(int i = 0;i < cnt;i++) {
        uint16_t b = popcnt(state & poly) & 1u;
        state = (state << 1u) | b;
        rtn += ('0' + b);
    }
    return rtn;
}

void print_binary(uint32_t v) {
    for(int k = 0;k < 32;k++) {
        bool b = (v >> (32-k-1)) & 1;
        fprintf(stderr,"%d", b );
    }
    fprintf(stderr,"\n");
}

int lfsr_order(lfsr_poly_t v) {
    int rtn = 1;
    v >>= 1;
    while(v) {
        rtn++;
        v >>= 1;
    }
    return rtn;
}

lfsr_t lsfr_iterate(lfsr_t lsfr, uint32_t cnt) {
    return lfsr_t(lsfr_iterate( lsfr.state,lsfr.poly, cnt), lsfr.poly);
}

uint32_t lfsr_find(lfsr_poly_t p, lfsr_state_t start, lfsr_state_t end) {
    uint32_t state = start;
    uint32_t period = lfsr_order(p);
    uint32_t mask = (1 << (period)) -1;
    uint32_t cnt = 0;
    do {
        cnt++;
        state = lsfr_iterate(state, p);
    } while( (end & mask) != (state & mask));
    return cnt;
}

uint32_t lfsr_period(lfsr_poly_t p) {
    return lfsr_find(p, 1, 1);
}

lfsr_poly_t lsfr_mirror_poly(lfsr_poly_t p) {
    uint32_t order = lfsr_order(p);
    lfsr_poly_t rtn = 1 << (order-1);
    for(uint8_t i = 0;i < order;i++) {
        if(p & (1u << (i-1)))
            rtn |= 1u << (order-i-1);
    }


    return rtn;
}

void lsfr_print(lfsr_state_t start, lfsr_poly_t p) {
    uint32_t state = start;
    uint32_t period = lfsr_order(p);
    uint32_t mask = (1 << (period)) -1;
    uint32_t cnt = 0;
    printf("0x%x 0x%2x: ", p, start);
    do {
        cnt++;
        state = lsfr_iterate(state, p);
        printf("%d", state & 1);
    } while( (start & mask) != (state & mask));
    printf("\n");
}

struct lfsr_lookup_t {
    uint32_t order;
    lfsr_poly_t p;
    uint32_t* table;
};

lfsr_lookup_t *lfsr_lookup_ctor(lfsr_poly_t p) {
    uint32_t order = lfsr_order(p);
    auto lookup = new lfsr_lookup_t();
    lookup->table = (uint32_t *)calloc(1 << order, sizeof(uint32_t));
    lookup->order = order;
    uint32_t start = 1;
    uint32_t state = start;
    uint32_t mask = (1 << (order)) -1;
    uint32_t cnt = 0;

    do {
        //if(cnt > order) {
            assert(lookup->table[state & mask] == 0);
            lookup->table[state & mask] = cnt;
        //}
        cnt++;
        state = lsfr_iterate(state, p);
    } while( (start & mask) != (state & mask));

    return lookup;
}

uint32_t lfsr_lookup_query(lfsr_lookup_t *lookup, uint32_t q) {
    uint32_t mask = (1 << (lookup->order)) -1;
    return lookup->table[q & mask];
}

uint32_t lfsr_find_with_mask(lfsr_poly_t p, lfsr_state_t start, lfsr_state_t state, uint32_t mask) {
    for(uint8_t j = 0;j < 16;j++) {
        if( ((mask >> j) & 0x1FFFF) == 0x1FFFF) {
            return lfsr_find(p, start, state >> j) - j;
        }
    }
    return 0;
}


lfsr_t::lfsr_t(lfsr_poly_t poly, lfsr_state_t state) : poly(poly), state(state) {}
