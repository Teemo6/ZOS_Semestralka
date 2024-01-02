#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <deque>

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

    Superblock *sb;
    Bitmap *inode_bitmap;
    Bitmap *data_bitmap;
    std::vector<IndexNode *> inode_vector;
    std::vector<std::array<unsigned char, CLUSTER_SIZE>> data_vector;

    Directory *curr_dir;

    void write_all();

    void free_inode(uint32_t id);
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> get_file_content(uint32_t inode);
    uint32_t get_directory_data_block(const std::string &path);
    std::array<std::string, 2> parse_path_and_name(std::string &path);

public:
    explicit FileSystem(std::string name);
    ~FileSystem();

    bool is_initialized() const;
    std::string get_name() const;

    void cp(const std::string &file1, const std::string &file2);
    void mv(std::string &source, std::string &dest);
    void rm(const std::string &file);
    void mkdir(std::string &dir_name);
    void rmdir(std::string &dir_name);
    void ls(const std::string &dir_name);
    void cat(std::string &file);
    void cd(const std::string &dir_name);
    void pwd();
    void info(std::string &file);
    void incp(const std::string &system, std::string &virt);
    void outcp(std::string &virt, const std::string &system);
    void ln(const std::string &file1, const std::string &file2);
    void format(uint32_t size);
};

#endif