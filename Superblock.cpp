#include <cstring>
#include <iostream>
#include "Superblock.hpp"



Superblock::Superblock(){
    strcpy(signature, "I-Node FS");

    std::cout << &signature[0] << std::endl;
}