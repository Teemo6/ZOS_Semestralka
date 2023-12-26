#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <string>
#include <fstream>

#include "Superblock.hpp"
#include "DirectoryItem.hpp"

class FileSystem {
private:
    bool initialized;
    std::string name;
    std::ofstream out_file;

public:
    explicit FileSystem(std::string name);
    ~FileSystem();


    Superblock *sb;



    void format(uint32_t size);



    bool is_initialized() const;
    std::string get_name() const;
};


#endif
