#ifndef ZOS_SEMESTRALKA_REFERENCEBLOCK_HPP
#define ZOS_SEMESTRALKA_REFERENCEBLOCK_HPP

#include <vector>
#include <array>
#include <cstdint>

#include "Constants.hpp"

class ReferenceBlock {
private:
    std::vector<std::uint32_t> references;

public:
    ReferenceBlock();
    explicit ReferenceBlock(std::array<unsigned char, CLUSTER_SIZE> &data);

    void add_reference(uint32_t ref);
    uint32_t get_reference(uint32_t pos) const;
    uint32_t get_reference_count() const;

    std::array<unsigned char, CLUSTER_SIZE> serialize();
};


#endif
