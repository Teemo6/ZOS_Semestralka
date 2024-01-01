#ifndef ZOS_SEMESTRALKA_REFERENCEBLOCK_HPP
#define ZOS_SEMESTRALKA_REFERENCEBLOCK_HPP

#include <vector>
#include <array>
#include <cstdint>

#include "Constants.hpp"

class ReferenceBlock {
private:
    uint32_t count;
    std::vector<std::uint32_t> references;

public:
    ReferenceBlock();
    explicit ReferenceBlock(std::array<unsigned char, CLUSTER_SIZE> &data);

    void add_reference(uint32_t ref);

    std::array<unsigned char, CLUSTER_SIZE> serialize();
};


#endif
