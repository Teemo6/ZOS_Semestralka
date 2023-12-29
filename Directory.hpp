#ifndef ZOS_SEMESTRALKA_DIRECTORY_HPP
#define ZOS_SEMESTRALKA_DIRECTORY_HPP

#include <vector>

#include "DirectoryItem.hpp"

class Directory {
private:

public:
    Directory(uint32_t self_inode, uint32_t parent_inode);
    Directory(std::array<unsigned char, CLUSTER_SIZE> &data);

    void add_file(const std::string &name, uint32_t inode);
    void remove_file(uint32_t inode);

    std::array<unsigned char, CLUSTER_SIZE> serialize();

    uint32_t self;
    uint32_t parent;
    std::vector<DirectoryItem *> content;
};

#endif
