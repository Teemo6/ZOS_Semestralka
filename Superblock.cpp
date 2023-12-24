#include <cstring>
#include <iostream>
#include <sstream>

#include "Superblock.hpp"



Superblock::Superblock(){
    strncpy((char *)signature, "", sizeof(signature));
    disk_size = 0;
    cluster_size = 0;
    cluster_count = 0;
    inode_count = 0;
    inode_bitmap_start = 0;
    data_bitmap_start = 0;
    inode_block_start = 0;
    data_block_start = 0;
}

void Superblock::init(uint32_t size){
    strncpy((char *)signature, "farag844", sizeof(signature) - 1);
    signature[sizeof(signature) - 1] = '\0';

    disk_size = size;
    cluster_size = CLUSTER_SIZE;
    cluster_count = (size - sizeof(Superblock)) / CLUSTER_SIZE;
    inode_count = INODE_COUNT;
    inode_bitmap_start = sizeof(Superblock);
    data_bitmap_start = inode_bitmap_start + sizeof(uint32_t);
    inode_block_start = data_bitmap_start + sizeof(uint32_t);
    data_block_start = inode_block_start + sizeof(uint32_t);
}

std::string Superblock::to_string(){
    std::stringstream ss;

    ss << "author: " << signature
       << ", disk_size: " << disk_size
       << ", cluster_size: " << cluster_size
       << ", cluster_count: " << cluster_count
       << ", inode_count: " << inode_count
       << ", inode_bitmap_start: " << inode_bitmap_start
       << ", data_bitmap_start: " << data_bitmap_start
       << ", inode_block_start: " << inode_block_start
       << ", data_block_start: " << data_block_start;

    return ss.str();
}