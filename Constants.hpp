#ifndef ZOS_SEMESTRALKA_CONSTANTS_HPP
#define ZOS_SEMESTRALKA_CONSTANTS_HPP

#include <unordered_map>

const int STRING_LENGHT = 12;

const int CLUSTER_SIZE = 1024;
const int INODE_COUNT = 1000;




const int MOST_COMMAND_ARG = 3;
const std::unordered_map<std::string, int> COMMANDS = {
        {"cp",      3},
        {"mv",      3},
        {"rm",      3},
        {"mkdir",   2},
        {"rmdir",   3},
        {"cat",     2},
        {"cd",      2},
        {"pwd",     1},
        {"info",    2},
        {"incp",    3},
        {"outcp",   3},
        {"load",    2},
        {"format",  2},
        {"ln",      3},
};

#endif
