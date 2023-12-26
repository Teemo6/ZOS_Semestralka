#include <iostream>
#include <sstream>
#include <csignal>
#include <vector>

#include "Constants.hpp"
#include "FileSystem.hpp"

FileSystem *fs = nullptr;

void end(int id){
    delete fs;
    std::cout << "Closing application" << std::endl;
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

    // Init fs
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
            std::cout << "cau" << std::endl;
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

            std::cout << fs->sb->to_string() << std::endl;
            fs->format(size);
            std::cout << fs->sb->to_string() << std::endl;
            continue;
        }
    }

    return EXIT_SUCCESS;
}
