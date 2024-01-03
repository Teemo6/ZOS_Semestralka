#include <iostream>
#include <csignal>

#include "InputParser.hpp"
#include "FileSystem.hpp"

FileSystem *fs;

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

    // Init file system
    fs = new FileSystem(argv[1]);

    // Input loop
    while (true){
        std::string line;
        std::cout << fs->get_name() << "> ";
        std::getline(std::cin, line);

        // Parse input
        if (!InputParser::parse_input(line, fs)) break;
    }
    end(1);
}
