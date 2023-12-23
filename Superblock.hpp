#ifndef ZOS_SEMESTRALKA_SUPERBLOCK_HPP
#define ZOS_SEMESTRALKA_SUPERBLOCK_HPP

#include <cstdint>

class Superblock {
private:

public:
    Superblock();


    char signature[10];             //login autora FS
};

#endif
