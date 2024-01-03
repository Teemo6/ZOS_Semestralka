#include <sstream>

#include "IndexNode.hpp"

IndexNode::IndexNode(uint32_t id) : node_id(id){
    is_directory = FALSE;
    references = INVALID;
    raw_size = INVALID;

    direct1 = INVALID;
    direct2 = INVALID;
    direct3 = INVALID;
    direct4 = INVALID;
    direct5 = INVALID;
    indirect1 = INVALID;
    indirect2 = INVALID;
}

void IndexNode::reset(){
    is_directory = FALSE;
    references = INVALID;
    raw_size = INVALID;

    direct1 = INVALID;
    direct2 = INVALID;
    direct3 = INVALID;
    direct4 = INVALID;
    direct5 = INVALID;
    indirect1 = INVALID;
    indirect2 = INVALID;
}

void IndexNode::set_directory(uint32_t block){
    is_directory = TRUE;
    direct1 = block;
}

bool IndexNode::write_direct_data(uint32_t value){
    if (is_directory) return false;

    if (direct1 == INVALID) direct1 = value;
    else if (direct2 == INVALID) direct2 = value;
    else if (direct3 == INVALID) direct3 = value;
    else if (direct4 == INVALID) direct4 = value;
    else if (direct5 == INVALID) direct5 = value;
    else return false;
    return true;
}

std::string IndexNode::occupied_blocks() const{
    int count = 0;

    if (direct1 != INVALID) count++;
    if (direct2 != INVALID) count++;
    if (direct3 != INVALID) count++;
    if (direct4 != INVALID) count++;
    if (direct5 != INVALID) count++;

    return std::to_string(count);
}

std::string IndexNode::to_string() const{
    std::stringstream ss;

    ss << "id: " << node_id
       << ", directory: " << is_directory
       << ", references: " << references
       << ", direct1: " << direct1
       << ", direct2: " << direct2
       << ", direct3: " << direct3
       << ", direct4: " << direct4
       << ", direct5: " << direct5
       << ", indirect1: " << indirect1
       << ", indirect2: " << indirect2;

    return ss.str();
}