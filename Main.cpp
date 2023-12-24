#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Constants.hpp"
#include "FileSystem.hpp"

int main(int argc, char *argv[]){
    if(argc != 2){
        std::cout << "Usage: ./ZOS_Semestralka <file system name>" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto *fs = new FileSystem(argv[1]);
    std::string fs_name = fs->get_name();

    std::cout << "FS: " << sizeof(FileSystem) << std::endl;
    std::cout << "Superblock: " << sizeof(Superblock) << std::endl;
    std::cout << "DirItem: " << sizeof(DirectoryItem) << std::endl;

    auto *sb1 = new Superblock();
    std::cout << fs->sb->to_string() << std::endl;

    std::ofstream outFile("fs.ext", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening the file for writing!" << std::endl;
        return 1;
    }
    outFile.write(reinterpret_cast<const char*>(fs->sb), sizeof(Superblock));
    outFile.close();

    std::cout << "-------------------" << std::endl;
    std::cout << sb1->to_string() << std::endl;

    std::ifstream inFile("fs.ext", std::ios::binary);
    if (!inFile) {
        std::cerr << "Error opening the file for reading!" << std::endl;
        return 1;
    }
    inFile.read(reinterpret_cast<char*>(sb1), sizeof(Superblock));
    inFile.close();

    std::cout << "-------------------" << std::endl;
    std::cout << sb1->to_string() << std::endl;



    delete sb1;


    // Loop
    while (true){
        bool valid = false;
        std::string line;

        // Get input
        std::cout << fs_name << "> ";
        std::getline(std::cin, line);

        // Build command vector
        std::istringstream iss(line);
        std::vector<std::string> command;
        std::string part;
        while (iss >> part){
            command.push_back(part);
        }

        // Skip or exit
        if (command.empty()) continue;
        if (command[0] == "exit"){
            delete fs;
            break;
        }

        // Valid command
        if (COMMANDS.count(command[0]) != 0){
            if (command.size() != COMMANDS.at(command[0])){
                std::cout << "INVALID NUMBER OF PARAMETERS, EXPECTED " << std::to_string(COMMANDS.at(command[0]) - 1) << ", GOT " << command.size() - 1 << std::endl;
                continue;
            }
            valid = true;
        }
        if (command[0] == "ls" && (command.size() == 1 || command.size() == 2)){
            std::cout << "cau" << std::endl;
            valid = true;
        }

        if (!valid){
            std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
            continue;
        }
    }

    return EXIT_SUCCESS;
}
