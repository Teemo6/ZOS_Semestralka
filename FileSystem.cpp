#include "FileSystem.hpp"

#include <utility>
#include <iostream>

FileSystem::FileSystem(std::string new_name){
    name = std::move(new_name);
    sb = new Superblock();

    std::ifstream in_file(name.c_str(), std::ios::binary);
    if (in_file.is_open()){
        initialized = true;
        in_file.read(reinterpret_cast<char*>(sb), sizeof(Superblock));
        in_file.close();
    } else {
        initialized = false;
    }
    out_file.open(name.c_str(), std::ios::binary);

    if (!initialized) std::cout << "File system not initialized, call 'format' first" << std::endl;
}

FileSystem::~FileSystem(){
    std::cout << "Saving to file" << std::endl;

    if (!out_file.is_open()) out_file.open(name.c_str(), std::ios::binary);
    out_file.write(reinterpret_cast<const char*>(sb), sizeof(Superblock));
    out_file.close();
}

void FileSystem::format(uint32_t size){
    sb->init(size);
    initialized = true;
}



















bool FileSystem::is_initialized() const {
    return initialized;
}

std::string FileSystem::get_name() const{
    return name;
}