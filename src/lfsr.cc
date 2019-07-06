#include "lfsr.h"

lfsr_state_t lsfr_iterate(lfsr_state_t state, lfsr_poly_t poly, uint32_t cnt) {
    for(int i = 0;i < cnt;i++) {
        uint16_t b = popcnt(state & poly) & 1;
        state = (state << 1) | b;
    }
    return state;
}

void print_binary(uint32_t v) {
    for(int k = 0;k < 32;k++) {
        bool b = (v >> (32-k-1)) & 1;
        printf("%d", b );
    }
    printf("\n");
}

int poly_order(int32_t v) {
    int rtn = 0;
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
    uint32_t period = poly_order(p);
    uint32_t mask = (1 << (period + 1)) -1;
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

lfsr_t::lfsr_t(lfsr_poly_t poly, lfsr_state_t state) : poly(poly), state(state) {}
