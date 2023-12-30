#ifndef ZOS_SEMESTRALKA_CONSTANTS_HPP
#define ZOS_SEMESTRALKA_CONSTANTS_HPP

#include <unordered_map>

const uint32_t STRING_LENGHT = 12;

const uint32_t FREE = UINT32_MAX;
const uint32_t INVALID = UINT32_MAX - 1;

const std::string EMPTY_STRING = "";

const uint32_t CLUSTER_SIZE = 512;
const uint32_t INODE_COUNT = 1024;

const uint32_t BITMAP_BITS = 32;

const std::string CALL_FORMAT_MESSAGE = "File system not initialized, call 'format' first";




const int MOST_COMMAND_ARG = 3;
const std::unordered_map<std::string, int> COMMANDS = {
        {"cp",      3},
        {"mv",      3},
        {"rm",      3},
        {"mkdir",   2},
        {"rmdir",   2},
        {"cat",     2},
        {"cd",      2},
        {"pwd",     1},
        {"info",    2},
        {"incp",    3},
        {"outcp",   3},
        {"load_bits",    2},
        {"format",  2},
        {"ln",      3},
};

#endif
