#include "FileSystem.hpp"
#include "IndexNode.hpp"

#include <utility>
#include <iostream>

FileSystem::FileSystem(std::string new_name){
    name = std::move(new_name);

    sb = new Superblock();

    std::ifstream in_file(name.c_str(), std::ios::binary);
    if (in_file.is_open()){
        initialized = true;
        uint32_t ints, bits;

        // Superblock
        in_file.read(reinterpret_cast<char*>(sb), sizeof(Superblock));

        // Inode bitmap
        inode_bitmap = new Bitmap(sb->inode_count);
        ints = (sb->inode_count + BITMAP_BITS - 1) / BITMAP_BITS;
        for (int i = 0; i < ints; i++){
            in_file.read(reinterpret_cast<char*>(&bits), sizeof(bits));
            inode_bitmap->load_bits(i, bits);
        }

        // Data bitmap
        data_bitmap = new Bitmap(sb->cluster_count);
        ints = (sb->cluster_count + BITMAP_BITS - 1) / BITMAP_BITS;
        for (int i = 0; i < ints; i++){
            in_file.read(reinterpret_cast<char*>(&bits), sizeof(bits));
            data_bitmap->load_bits(i, bits);
        }

        std::cout << "Superblock: " << sb->to_string() << std::endl;
        std::cout << "Inode bit: " << inode_bitmap->to_string() << std::endl;
        std::cout << "Data bit: " << data_bitmap->to_string() << std::endl;

        in_file.close();
    } else {
        initialized = false;
    }

    if (!initialized){
        std::cout << "File system not initialized, call 'format' first" << std::endl;
    }
}

FileSystem::~FileSystem(){
    std::cout << "Saving to file" << std::endl;
    write_all();
    out_file.close();
}

void FileSystem::write_all(){
    uint32_t ints;

    if (!out_file.is_open()){
        out_file.open(name.c_str(), std::ios::binary);
    }
    out_file.seekp(0, std::ios::beg);

    // Superblock
    out_file.write(reinterpret_cast<const char*>(sb), sizeof(Superblock));

    // Inode bitmap
    ints = (sb->inode_count + BITMAP_BITS - 1) / BITMAP_BITS;
    for(int i = 0; i < ints; i++){
        uint32_t bits = inode_bitmap->get_bits(i);
        out_file.write(reinterpret_cast<const char*>(&bits), sizeof(uint32_t));
    }

    // Data bitmap
    ints = (sb->cluster_count + BITMAP_BITS - 1) / BITMAP_BITS;
    for(uint32_t i = 0; i < ints; i++){
        uint32_t bits = data_bitmap->get_bits(i);
        out_file.write(reinterpret_cast<const char*>(&bits), sizeof(uint32_t));
    }
}

void FileSystem::format(uint32_t size){
    delete sb;
    delete inode_bitmap;
    delete data_bitmap;

    sb = new Superblock();
    sb->init(size);

    inode_bitmap = new Bitmap(sb->inode_count);
    data_bitmap = new Bitmap(sb->cluster_count);

    initialized = true;

    write_all();
}


void FileSystem::get_inode(){
    for(int i = 0; i < 20; i++){
        inode_bitmap->get_free();
        data_bitmap->get_free();
    }
}

void FileSystem::free_inode(uint32_t size){
    inode_bitmap->set_free(size);
    data_bitmap->set_free(size);
}


















bool FileSystem::is_initialized() const {
    return initialized;
}

std::string FileSystem::get_name() const{
    return name;
}