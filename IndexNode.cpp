#include <sstream>

#include "IndexNode.hpp"

IndexNode::IndexNode(uint32_t id) : node_id(id){
    is_directory = false;
    references = INVALID;
    file_size = INVALID;

    direct1 = INVALID;
    direct2 = INVALID;
    direct3 = INVALID;
    direct4 = INVALID;
    direct5 = INVALID;
    indirect1 = INVALID;
    indirect2 = INVALID;
}

void IndexNode::reset(){
    is_directory = false;
    references = INVALID;
    file_size = INVALID;

    direct1 = INVALID;
    direct2 = INVALID;
    direct3 = INVALID;
    direct4 = INVALID;
    direct5 = INVALID;
    indirect1 = INVALID;
    indirect2 = INVALID;
}

void IndexNode::set_directory(uint32_t block){
    is_directory = true;
    file_size = CLUSTER_SIZE;
    direct1 = block;
}

std::string IndexNode::occupied_blocks() const{
    std::stringstream ss;

    if (direct1 != INVALID) ss << "[" << direct1 << "] ";
    if (direct2 != INVALID) ss << "[" << direct2 << "] ";
    if (direct3 != INVALID) ss << "[" << direct3 << "] ";
    if (direct4 != INVALID) ss << "[" << direct4 << "] ";
    if (direct5 != INVALID) ss << "[" << direct5 << "] ";

    return ss.str();
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