#include "IndexNode.hpp"

IndexNode::IndexNode(uint32_t id, bool directory){
    node_id = id;
    is_directory = directory;

    references = 0;
    file_size = 0;
    direct1 = 0;
    direct2 = 0;
    direct3 = 0;
    direct4 = 0;
    direct5 = 0;
    indirect1 = 0;
    indirect2 = 0;
}