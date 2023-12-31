#ifndef ZOS_SEMESTRALKA_INPUTPARSER_HPP
#define ZOS_SEMESTRALKA_INPUTPARSER_HPP

#include "FileSystem.hpp"

class InputParser {
private:
    static bool loading;
    static void parse_load(const std::string &input, FileSystem *fs);

public:
    static bool parse_input(const std::string &input, FileSystem *fs);
};

#endif
