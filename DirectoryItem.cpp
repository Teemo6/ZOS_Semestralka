#include <cstring>

#include "DirectoryItem.hpp"

DirectoryItem::DirectoryItem(std::string &new_name, uint32_t new_inode){
    std::strncpy((char *)name, new_name.c_str(), sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    inode = new_inode;
    next = nullptr;
}