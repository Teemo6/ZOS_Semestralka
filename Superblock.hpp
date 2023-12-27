#ifndef ZOS_SEMESTRALKA_SUPERBLOCK_HPP
#define ZOS_SEMESTRALKA_SUPERBLOCK_HPP

#include <cstdint>
#include <string>

#include "Constants.hpp"

class Superblock {
private:

public:
    Superblock();

    void load();
    void init(uint32_t size);

    std::string to_string();



    unsigned char signature[STRING_LENGHT]{};
    uint32_t disk_size;
    uint32_t cluster_size;
    uint32_t inode_count;
    uint32_t cluster_count;
    uint32_t inode_bitmap_start;
    uint32_t data_bitmap_start;
    uint32_t inode_block_start;
    uint32_t data_block_start;
};

#endif
