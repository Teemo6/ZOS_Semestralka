#ifndef ZOS_SEMESTRALKA_INDEXNODE_H
#define ZOS_SEMESTRALKA_INDEXNODE_H

#include <cstdint>

#include "DirectoryItem.hpp"

class IndexNode{
private:


public:
    explicit IndexNode(uint32_t id);

    uint32_t node_id;
    uint32_t is_directory;
    uint32_t references;
    uint32_t raw_size;

    uint32_t direct1;
    uint32_t direct2;
    uint32_t direct3;
    uint32_t direct4;
    uint32_t direct5;
    uint32_t indirect1;
    uint32_t indirect2;

    void reset();
    void set_directory(uint32_t block);
    bool write_direct_data(uint32_t value);

    std::string occupied_blocks() const;
    std::string to_string() const;
};

#endif
