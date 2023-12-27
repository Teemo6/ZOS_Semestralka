#ifndef ZOS_SEMESTRALKA_CONSTANTS_HPP
#define ZOS_SEMESTRALKA_CONSTANTS_HPP

#include <unordered_map>

const uint32_t STRING_LENGHT = 12;

const uint32_t ID_ITEM_FREE = -1;
const uint32_t INVALID = -1;

const uint32_t CLUSTER_SIZE = 512;
const uint32_t INODE_COUNT = 1024;

const uint32_t BITMAP_BITS = 32;




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
        {"load_bits",    2},
        {"format",  2},
        {"ln",      3},
};

#endif
