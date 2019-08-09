#include "lh2polys.h"

std::tuple<uint32_t, uint32_t> parse_sample_mask(const std::string& s) {
    uint32_t sample = 0, mask = 0;

    for(char i : s) {
        sample <<= 1;
        mask <<= 1;
        if(i == '1')
            sample |= 1;
        if(i != '_')
            mask |= 1;
    }

    return std::make_tuple(sample, mask);
}

// Data from mode 7; roughly dead center in front of LH 3 ft away:
// 10100000000010101001110____0_0__ 0100001100111100_1101101101101__ 010011000110011001100010110_0___ 0011110111100010100011__01___0__ 010101111101000001100___000_____ 10010111001011111000011__1__0___ 10000011110110111010__10____1___
int main(int argc, char** argv) {
    for(int i = 1;i < argc;i++) {
        auto [sample, mask] = parse_sample_mask(argv[i]);
        int best = find_best_poly(sample, mask, 12, 14);
        printf("Best for '%s': %d -- %d d: %d\n", argv[i], best, lfsr_period(poly_pairs[best]), lfsr_find_with_mask(poly_pairs[best], 1, sample, mask));
    }
    return 0;
}