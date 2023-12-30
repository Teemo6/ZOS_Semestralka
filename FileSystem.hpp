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
    std::string fs_name;
    std::ofstream out_file;

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

    bool is_initialized() const;
    std::string get_name() const;

    void free_inode(uint32_t id);
    uint32_t get_dir_from_path(const std::string &path);

    void cp(const std::string &file1, const std::string &file2);
    void mv(const std::string &file1, const std::string &file2);
    void rm(const std::string &file);
    void mkdir(const std::string &dir_name);
    void rmdir(const std::string &dir_name);
    void ls(const std::string &dir_name);
    void cat(const std::string &file);
    void cd(const std::string &dir_name);
    void pwd();
    void info(const std::string &file);
    void incp(const std::string &file1, const std::string &file2);
    void outcp(const std::string &file1, const std::string &file2);
    void load(const std::string &file);
    void format(uint32_t size);
};


#endif
