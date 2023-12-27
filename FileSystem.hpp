#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <string>
#include <fstream>
#include <vector>

#include "Superblock.hpp"
#include "DirectoryItem.hpp"
#include "IndexNode.hpp"
#include "Bitmap.hpp"

class FileSystem {
private:
    bool initialized;
    std::string name;
    std::ofstream out_file;

public:
    explicit FileSystem(std::string name);
    ~FileSystem();


    Superblock *sb;
    Bitmap *inode_bitmap;
    Bitmap *data_bitmap;
    std::vector<IndexNode*> inodes;

    void write_all();

    void format(uint32_t size);
    void get_inode();
    void free_inode(uint32_t size);



    bool is_initialized() const;
    std::string get_name() const;
};


#endif
