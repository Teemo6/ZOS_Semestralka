#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <string>
#include <fstream>
#include <vector>

#include "Superblock.hpp"
#include "DirectoryItem.hpp"
#include "Directory.hpp"
#include "IndexNode.hpp"
#include "Bitmap.hpp"

class FileSystem {
private:
    bool initialized;
    std::string name;
    std::ofstream out_file;

    Directory *root_dir;
    Directory *curr_dir;

    void write_all();

public:
    explicit FileSystem(std::string name);
    ~FileSystem();


    Superblock *sb;
    Bitmap *inode_bitmap;
    Bitmap *data_bitmap;
    std::vector<IndexNode *> inode_vector;
    std::vector<std::array<unsigned char, CLUSTER_SIZE>> data_vector;



    void mkdir(const std::string &dir_name);
    void ls(const std::string &dir_name);
    void format(uint32_t size);

    void get_inode();
    void free_inode(uint32_t size);

    bool is_initialized() const;
    std::string get_name() const;
};


#endif
