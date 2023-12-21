#include <iostream>
#include <sstream>
#include <vector>

#include "Constants.hpp"

// Argument definition
#define ARG_COUNT 2
#define ARG_FILE_SYSTEM   argv[1]

int main(int argc, char *argv[]){
    if(argc != ARG_COUNT){
        std::cout << "Usage:\n\t./ZOS_Semestralka <name of filesystem>\n";
        exit(EXIT_FAILURE);
    }

    std::string fs_name(ARG_FILE_SYSTEM);

    // Loop
    std::string line;

    while (true){
        std::cout << fs_name << "> ";
        std::getline(std::cin, line);

        std::istringstream iss(line);
        std::vector<std::string> command;
        std::string part;

        while (iss >> part){
            command.push_back(part);
        }

        if (command.empty()) continue;

        if (COMMANDS.find(command[0]) != COMMANDS.end()){
            if (command.size() != COMMANDS.at(command[0])){
                std::cout << "INVALID NUMBER OF PARAMETERS, EXPECTED " << std::to_string(COMMANDS.at(command[0])) << ", GOT " << command.size() << std::endl;
                continue;
            }
        }

        if (command[0] == "ls" && (command.size() == 1 || command.size() == 2)){

        }

        if (command[0] == "exit"){
            break;
        }
    }

    return EXIT_SUCCESS;
}
