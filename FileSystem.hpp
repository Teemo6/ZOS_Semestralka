#ifndef ZOS_SEMESTRALKA_FILESYSTEM_H
#define ZOS_SEMESTRALKA_FILESYSTEM_H

#include <deque>
#include <fstream>

#include "Superblock.hpp"
#include "Bitmap.hpp"
#include "IndexNode.hpp"
#include "Directory.hpp"
#include "ReferenceBlock.hpp"

class FileSystem {
private:
    bool initialized;
    std::string fs_name;
    std::ofstream out_file;
    Directory *curr_dir;

    Superblock *sb;
    Bitmap *inode_bitmap;
    Bitmap *data_bitmap;
    std::vector<IndexNode *> inode_vector;
    std::vector<std::array<unsigned char, CLUSTER_SIZE>> data_vector;

    void write_all();

    void update_curr_dir_if_same(uint32_t id);
    void free_directory_inode(uint32_t id);
    std::array<unsigned char, CLUSTER_SIZE> create_empty_data_block(uint32_t id);
    std::deque<uint32_t> get_file_data_blocks(uint32_t inode);
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> get_file_content(uint32_t inode);
    uint32_t get_directory_data_block(const std::string &path);
    std::array<std::string, 2> parse_path_and_name(std::string &path);

public:
    explicit FileSystem(std::string name);
    ~FileSystem();

    bool is_initialized() const;
    std::string get_name() const;

    void cp(std::string &source, std::string &dest);
    void mv(std::string &source, std::string &dest);
    void rm(std::string &file);
    void mkdir(std::string &dir_name);
    void rmdir(std::string &dir_name);
    void ls(const std::string &dir_name);
    void cat(std::string &file);
    void cd(const std::string &dir_name);
    void pwd();
    void info(std::string &file);
    void incp(const std::string &system, std::string &virt);
    void outcp(std::string &virt, const std::string &system);
    void ln(std::string &source, std::string &link);
    void format(uint32_t size);
};

#endif