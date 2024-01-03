#ifndef ZOS_SEMESTRALKA_DIRECTORY_HPP
#define ZOS_SEMESTRALKA_DIRECTORY_HPP

#include <vector>
#include <array>

#include "DirectoryItem.hpp"

class Directory {
public:
    Directory(uint32_t self_inode, uint32_t parent_inode);
    explicit Directory(std::array<unsigned char, CLUSTER_SIZE> &data);
    ~Directory();

    bool is_empty();
    void add_file(const std::string &name, uint32_t inode);
    void remove_file(const std::string &name);
    void update_parent(uint32_t new_inode);

    uint32_t get_file_inode(const std::string &name);
    std::string get_file_name(uint32_t inode);

    std::array<unsigned char, CLUSTER_SIZE> serialize();

    uint32_t self;
    uint32_t parent;
    std::vector<DirectoryItem *> content;
};

#endif
