#ifndef ZOS_SEMESTRALKA_INDEXNODE_H
#define ZOS_SEMESTRALKA_INDEXNODE_H

#include <cstdint>

#include "DirectoryItem.hpp"

class IndexNode{
private:


public:
    explicit IndexNode(uint32_t id);

    uint32_t node_id;
    bool is_directory;
    uint32_t references;
    uint32_t file_size;

    uint32_t direct1;
    uint32_t direct2;
    uint32_t direct3;
    uint32_t direct4;
    uint32_t direct5;
    uint32_t indirect1;
    uint32_t indirect2;

    void reset();
    void set_directory(uint32_t block);

    std::string to_string() const;
};

#endif
