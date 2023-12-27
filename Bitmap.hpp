#ifndef ZOS_SEMESTRALKA_BITMAP_HPP
#define ZOS_SEMESTRALKA_BITMAP_HPP

#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <bitset>

#include "Constants.hpp"

class Bitmap{
    uint32_t bit_count;
    std::vector<std::bitset<BITMAP_BITS>> data;

public:
    Bitmap(uint32_t count);
    void load_bits(int set, uint32_t bits);

    uint32_t get_bits(uint32_t pos);

    uint32_t get_free();
    void set_free(uint32_t pos);

    std::string to_string();

};

#endif