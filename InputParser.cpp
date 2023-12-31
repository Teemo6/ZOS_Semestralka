#include <iostream>
#include <sstream>
#include <vector>

#include "InputParser.hpp"
#include "Constants.hpp"
#include "FileSystem.hpp"

bool InputParser::loading = false;

bool InputParser::parse_input(const std::string &input, FileSystem *fs){
    std::istringstream iss(input);
    std::vector<std::string> command;
    std::string part;
    while (iss >> part){
        command.push_back(part);
    }

    // Skip or exit
    if (command.empty()) return true;
    if (command[0] == "exit") return false;

    // Valid command
    bool valid = false;
    if (COMMANDS.count(command[0]) != 0){
        if (command.size() != COMMANDS.at(command[0])){
            std::cout << "Invalid number of parameters, expected " << std::to_string(COMMANDS.at(command[0]) - 1) << ", got " << command.size() - 1 << std::endl;
            return true;
        }
        valid = true;
    }
    if (command[0] == "ls" && (command.size() == 1 || command.size() == 2)){
        valid = true;
    }

    if (!valid){
        std::cout << "Invalid command '" << command[0] << "'" << std::endl;
        return true;
    }

    // Handle command
    if (command[0] == "mv") fs->mv(command[1], command[2]);
    else if (command[0] == "mkdir") fs->mkdir(command[1]);
    else if (command[0] == "rmdir") fs->rmdir(command[1]);
    else if (command[0] == "ls" && command.size() == 1) fs->ls("");
    else if (command[0] == "ls" && command.size() == 2) fs->ls(command[1]);
    else if (command[0] == "cd") fs->cd(command[1]);
    else if (command[0] == "pwd") fs->pwd();
    else if (command[0] == "info") fs->info(command[1]);
    else if (command[0] == "load"){
        if (!loading) {
            loading = true;
            parse_load(command[1], fs);
        } else {
            std::cout << "Already loading from file" << std::endl;
        }
    }
    else if (command[0] == "format"){
        uint32_t size;
        try{
            size = std::stoi(command[1]);
        } catch (const std::exception &e){
            std::cout << "Invalid parameter '" << command[1] << "'" << std::endl;
            return true;
        }
        fs->format(size);
    }
    return true;
}

//////////////////////////////////

void InputParser::parse_load(const std::string &input, FileSystem *fs){
    std::ifstream file(input);
    if (!file.is_open()){
        std::cerr << "File not found" << std::endl;
        loading = false;
        return;
    }
    std::cout << "Ok" << std::endl;

    std::string line;
    while (std::getline(file, line)){
        std::cout << line << std::endl;
        parse_input(line, fs);
    }
    loading = false;
    file.close();
}