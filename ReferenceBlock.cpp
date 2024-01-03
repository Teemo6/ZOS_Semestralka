#include "ReferenceBlock.hpp"

ReferenceBlock::ReferenceBlock(){
    references = std::vector<uint32_t>();
}

ReferenceBlock::ReferenceBlock(std::array<unsigned char, CLUSTER_SIZE> &data){
    references = std::vector<uint32_t>();

    for (int i = 0; i < data.size(); i += sizeof(uint32_t)) {
        uint32_t ref = 0;
        for (int j = 0; j < sizeof(uint32_t); j++) {
            ref |= static_cast<uint32_t>(data[i + j]) << (8 * j);
        }
        if (ref == 0) break;
        references.emplace_back(ref);
    }
}

void ReferenceBlock::add_reference(uint32_t ref){
    if (references.size() < CLUSTER_SIZE / sizeof(uint32_t)){
        references.emplace_back(ref);
    }
}

uint32_t ReferenceBlock::get_reference(uint32_t pos) const{
    if (pos < CLUSTER_SIZE / sizeof(uint32_t)) {
        return references[pos];
    }
    return INVALID;
}

uint32_t ReferenceBlock::get_reference_count() const{
    return references.size();
}

std::array<unsigned char, CLUSTER_SIZE> ReferenceBlock::serialize(){
    std::array<unsigned char, CLUSTER_SIZE> data = {};
    data.fill('\0');

    for (int i = 0; i < references.size(); i++){
        uint32_t ref = references[i];
        std::copy(reinterpret_cast<const unsigned char*>(&ref),reinterpret_cast<const unsigned char*>(&ref) + sizeof(uint32_t),data.data() + i * sizeof(uint32_t));
    }
    return data;
}

