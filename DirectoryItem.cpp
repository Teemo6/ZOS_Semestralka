#include <cstring>
#include <sstream>

#include "DirectoryItem.hpp"

DirectoryItem::DirectoryItem(const std::string &new_name, uint32_t new_inode){
    std::strncpy((char *)name, new_name.c_str(), sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    inode = new_inode;
}

std::string DirectoryItem::to_string(){
    std::stringstream ss;

    ss << "inode: " << inode
       << ", fs_name: " << name;

    return ss.str();
}