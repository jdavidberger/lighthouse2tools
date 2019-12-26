#include <iostream>
#include "lh2polys.h"

static std::tuple<uint32_t, uint32_t> parse_sample_mask(const std::string& s) {
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

int main(int argc, char** argv) {
    auto [sample, mask] = parse_sample_mask(argv[1]);
    auto length = atoi(argv[2]);
    auto poly = 0x00018A55;
    std::cout << lsfr_iterate_str(sample, poly, length) << std::endl;
    return 0;
}