#include <iostream>
#include <bitset>
#include <sstream>

#include "Bitmap.hpp"

Bitmap::Bitmap(uint32_t count){
    bit_count = count;

    uint32_t vector_size = (count + BITMAP_BITS - 1) / BITMAP_BITS;
    data = std::vector<std::bitset<BITMAP_BITS>>(vector_size, std::bitset<BITMAP_BITS>(0));
}

void Bitmap::load_bits(uint32_t set, uint32_t bits){
    data[set] = std::bitset<BITMAP_BITS>(bits);
}

uint32_t Bitmap::get_bits(uint32_t pos){
    return static_cast<uint32_t>(data[pos].to_ulong());
}

uint32_t Bitmap::get_free(){
    uint32_t pos;
    for (int set = 0; set < data.size(); set++){
        for (int bit = 0; bit < BITMAP_BITS; bit++){
            pos = set * BITMAP_BITS + bit;
            if (pos >= bit_count){
                return INVALID;
            }
            if (!data[set][bit]){
                data[set][bit] = true;
                std::cout << "assigned " << pos << std::endl;
                return pos;
            }
        }
    }
    return INVALID;
}

void Bitmap::set_free(uint32_t pos){
    if (pos < bit_count){
        uint32_t set = pos / BITMAP_BITS;
        uint32_t bit = pos % BITMAP_BITS;
        data[set][bit] = false;
        std::cout << "set " << pos << " free" << std::endl;
    }
}

std::string Bitmap::to_string(){
    std::stringstream ss;

    for (auto bits : data) {
        ss << bits << " ";
    }

    return ss.str();
}