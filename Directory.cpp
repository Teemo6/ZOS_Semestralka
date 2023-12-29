#include <cstring>
#include <iostream>
#include <algorithm>

#include "Directory.hpp"

Directory::Directory(uint32_t self_inode, uint32_t parent_inode){
    self = self_inode;
    parent = parent_inode;
    content = std::vector<DirectoryItem *>();

    content.emplace_back(new DirectoryItem(std::string("."), self));
    content.emplace_back(new DirectoryItem(std::string(".."), self));
}

Directory::Directory(std::array<unsigned char, CLUSTER_SIZE> &data){
    content = std::vector<DirectoryItem *>();

    int offset = 0;
    for (int i = 0; i < CLUSTER_SIZE / sizeof(DirectoryItem); i++){
        std::array<unsigned char, sizeof(DirectoryItem)> row = {};
        std::memcpy(row.data(), data.data() + offset, sizeof(DirectoryItem));

        if (std::all_of(row.begin(), row.end(), [](unsigned char c) {return c == 0;})) break;

        auto *dir_item = new DirectoryItem("1", 1);
        std::memcpy(dir_item->name, row.data(), sizeof(dir_item->name));
        std::memcpy(&(dir_item->inode), row.data() + sizeof(dir_item->name), sizeof(uint32_t));

        content.emplace_back(dir_item);
        offset += sizeof(DirectoryItem);
    }

    self = content[0]->inode;
    parent = content[1]->inode;
}

void Directory::add_file(const std::string &name, uint32_t inode){
    content.emplace_back(new DirectoryItem(name, inode));
}

void Directory::remove_file(uint32_t inode){
    int to_remove = -1;
    for (int i = 2; i < content.size(); i++){
        auto *file = content[i];
        if (file->inode == inode){
            to_remove = i;
            break;
        }
    }
    if (to_remove != -1) content.erase(content.begin() + to_remove);
}

std::array<unsigned char, CLUSTER_SIZE> Directory::serialize(){
    std::array<unsigned char, CLUSTER_SIZE> data = {};
    data.fill('\0');

    int offset = 0;
    for(auto file : content){
        std::memcpy(data.data() + offset, file->name, sizeof(file->name));
        std::memcpy(data.data() + offset + sizeof(file->name), &(file->inode), sizeof(uint32_t));
        offset += sizeof(DirectoryItem);
    }
    return data;
}
