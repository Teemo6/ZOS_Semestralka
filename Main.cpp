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
        std::cout << "Usage: ./ZOS_Semestralka <file system fs_name>" << std::endl;
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
                std::cout << "Invalid number of parameters, expected " << std::to_string(COMMANDS.at(command[0]) - 1) << ", got " << command.size() - 1 << std::endl;
                continue;
            }
            valid = true;
        }
        if (command[0] == "ls" && (command.size() == 1 || command.size() == 2)){
            valid = true;
        }

        if (!valid){
            std::cout << "Invalid command '" << command[0] << "'" << std::endl;
            continue;
        }

        // Handle command
        if (command[0] == "mkdir"){
            fs->mkdir(command[1]);
            continue;
        }

        if (command[0] == "rmdir"){
            fs->rmdir(command[1]);
            continue;
        }

        if (command[0] == "ls"){
            if (command.size() == 1) fs->ls("");
            else fs->ls(command[1]);
            continue;
        }

        if (command[0] == "cd"){
            fs->cd(command[1]);
            continue;
        }

        if (command[0] == "pwd"){
            fs->pwd();
            continue;
        }

        if (command[0] == "format"){
            uint32_t size;
            try{
                size = std::stoi(command[1]);
            } catch (const std::exception &e){
                std::cout << "Invalid parameter '" << command[1] << "'" << std::endl;
                continue;
            }
            fs->format(size);
            continue;
        }
    }

    return EXIT_SUCCESS;
}
