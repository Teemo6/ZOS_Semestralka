#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <string>

#include "Superblock.hpp"
#include "DirectoryItem.hpp"

class FileSystem {
private:
    std::string name;
    FILE *file;


public:
    explicit FileSystem(std::string name);
    ~FileSystem();


    Superblock *sb;



    void format(int32_t bits);










    bool is_formatted() const;
    std::string get_name() const;
};


#endif
