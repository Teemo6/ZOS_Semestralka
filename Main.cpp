#include <iostream>
#include <sstream>
#include <csignal>
#include <vector>

#include "Constants.hpp"
#include "FileSystem.hpp"

FileSystem *fs = nullptr;

void end(int id){
    delete fs;
    std::cout << "File system closed successfully" << std::endl;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    // Check arg
    if(argc != 2){
        std::cout << "Usage: ./ZOS_Semestralka <file system name>" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Handle signal
    signal(SIGINT, end);
    signal(SIGTERM, end);

    // Init file system
    fs = new FileSystem(argv[1]);

    // Loop
    while (true){
        bool valid = false;
        std::string line;

        // Get input
        std::cout << fs->get_name() << "> ";
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
            end(SIGTERM);
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
            valid = true;
        }

        if (!valid){
            std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
            continue;
        }

        if (command[0] == "format"){
            uint32_t size;
            try{
                size = std::stoi(command[1]);
            } catch (const std::exception &e){
                std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
                continue;
            }
            fs->format(size);
            continue;
        }

        if (command[0] == "ls"){
            fs->ls("");
            continue;
        }

        if (command[0] == "cd"){
            uint32_t size;
            try{
                size = std::stoi(command[1]);
            } catch (const std::exception &e){
                std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
                continue;
            }
            IndexNode *inode = fs->inode_vector[size];
            std::cout << inode->to_string() << std::endl;
            continue;
        }

        if (command[0] == "cat"){
            uint32_t size;
            try{
                size = std::stoi(command[1]);
            } catch (const std::exception &e){
                std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
                continue;
            }
            fs->free_inode(size);
            std::cout << fs->inode_bitmap->to_string() << std::endl;
            std::cout << fs->data_bitmap->to_string() << std::endl;
            continue;
        }

        if (command[0] == "incp"){
            uint32_t size;
            try{
                size = std::stoi(command[1]);
            } catch (const std::exception &e){
                std::cout << "INVALID COMMAND '" << command[0] << "'" << std::endl;
                continue;
            }
            std::array<unsigned char, CLUSTER_SIZE> asd = fs->data_vector[size];
            std::cout << asd.data() << std::endl;
            continue;
        }

        if (command[0] == "mkdir"){
            fs->mkdir(command[1]);
            continue;
        }
    }

    return EXIT_SUCCESS;
}
