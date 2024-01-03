#ifndef ZOS_SEMESTRALKA_BITMAP_HPP
#define ZOS_SEMESTRALKA_BITMAP_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <bitset>

#include "Constants.hpp"

class Bitmap{
    uint32_t bit_count;
    std::vector<std::bitset<BITMAP_BITS>> data;

public:
    explicit Bitmap(uint32_t count);
    void load_bits(uint32_t set, uint32_t bits);

    uint32_t get_bits(uint32_t pos);

    bool check_free(uint32_t count);
    uint32_t get_free();
    void set_free(uint32_t pos);

    std::string to_string();
};

#endif