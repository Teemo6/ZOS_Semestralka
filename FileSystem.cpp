#include "FileSystem.hpp"

#include <utility>

FileSystem::FileSystem(std::string new_name){
    name = std::move(new_name);

    if (FILE *temp = fopen(name.c_str(), "r")) {
        fclose(temp);
        file = fopen(name.c_str(), "wb+");
    } else file = nullptr;
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

void FileSystem::incp(){

}

void FileSystem::outcp(){

}