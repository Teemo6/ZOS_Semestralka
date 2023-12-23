#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <string>

#include "Superblock.hpp"

class FileSystem {
public:
    explicit FileSystem(std::string name);
    ~FileSystem();

    void format(int32_t bits);

    bool is_formatted() const;

    std::string name;
    FILE *file;

    Superblock *sb{};
};


#endif
