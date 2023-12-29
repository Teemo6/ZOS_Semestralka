#include <sstream>

#include "IndexNode.hpp"

IndexNode::IndexNode(uint32_t id){
    node_id = id;
    is_directory = false;
    references = FREE;
    file_size = FREE;

    direct1 = FREE;
    direct2 = FREE;
    direct3 = FREE;
    direct4 = FREE;
    direct5 = FREE;
    indirect1 = FREE;
    indirect2 = FREE;
}

void IndexNode::set_directory(uint32_t block){
    is_directory = true;
    file_size = CLUSTER_SIZE;
    direct1 = block;

    references = INVALID;
    direct2 = INVALID;
    direct3 = INVALID;
    direct4 = INVALID;
    direct5 = INVALID;
    indirect1 = INVALID;
    indirect2 = INVALID;
}

std::string IndexNode::to_string() const{
    std::stringstream ss;

    ss << "id: " << node_id
       << ", directory: " << is_directory
       << ", references: " << references
       << ", size: " << file_size
       << ", direct1: " << direct1
       << ", direct2: " << direct2
       << ", direct3: " << direct3
       << ", direct4: " << direct4
       << ", direct5: " << direct5
       << ", indirect1: " << indirect1
       << ", indirect2: " << indirect2;

    return ss.str();
}