#ifndef ZOS_SEMESTRALKA_DIRECTORYITEM_HPP
#define ZOS_SEMESTRALKA_DIRECTORYITEM_HPP

#include <cstdint>
#include <string>

#include "Constants.hpp"

class DirectoryItem{
public:
    DirectoryItem(std::string &new_name, uint32_t new_inode);


    unsigned char name[STRING_LENGHT]{};
    uint32_t inode;
};


#endif
