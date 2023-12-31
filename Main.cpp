#include <iostream>
#include <csignal>

#include "InputParser.hpp"
#include "FileSystem.hpp"

volatile sig_atomic_t exit_flag = 0;

void end(int id){
    exit_flag = 1;
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
    auto *fs = new FileSystem(argv[1]);

    // Loop
    while (!exit_flag){
        // Get input
        std::string line;
        std::cout << fs->get_name() << "> ";
        std::getline(std::cin, line);

        // Parse input
        if (!InputParser::parse_input(line, fs)) break;
    }

    delete fs;
    std::cout << "File system closed successfully" << std::endl;

    return EXIT_SUCCESS;
}
