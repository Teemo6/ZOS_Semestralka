#include "FileSystem.hpp"

#include <utility>

FileSystem::FileSystem(std::string new_name){
    name = std::move(new_name);

    if (FILE *temp = fopen(name.c_str(), "r")) {
        fclose(temp);
        file = fopen(name.c_str(), "wb+");
    } else file = nullptr;

    sb = new Superblock();
    sb->init(102400);
}

FileSystem::~FileSystem(){
    if (is_formatted()) fclose(file);
}

void FileSystem::format(int32_t size){
    file = fopen(name.c_str(), "wb+");
}



















bool FileSystem::is_formatted() const{
    return file != nullptr;
}

std::string FileSystem::get_name() const{
    return name;
}