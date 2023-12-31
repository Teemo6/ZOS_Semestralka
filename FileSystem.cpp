#include <utility>
#include <iostream>
#include <sstream>
#include <string>

#include "FileSystem.hpp"
#include "IndexNode.hpp"
#include "DirectoryItem.hpp"

FileSystem::FileSystem(std::string new_name){
    fs_name = std::move(new_name);

    sb = new Superblock();
    inode_bitmap = nullptr;
    data_bitmap = nullptr;
    curr_dir = nullptr;
    inode_vector = std::vector<IndexNode *>{};
    data_vector = std::vector<std::array<unsigned char, CLUSTER_SIZE>>{};

    std::ifstream in_file(fs_name.c_str(), std::ios::binary);
    if (in_file.is_open() && in_file.peek() != std::ifstream::traits_type::eof()){
        initialized = true;
        uint32_t ints, bits;

        // Superblock
        in_file.read(reinterpret_cast<char*>(sb), sizeof(Superblock));

        // Inode bitmap
        inode_bitmap = new Bitmap(sb->inode_count);
        ints = (sb->inode_count + BITMAP_BITS - 1) / BITMAP_BITS;
        for (uint32_t i = 0; i < ints; i++){
            in_file.read(reinterpret_cast<char*>(&bits), sizeof(bits));
            inode_bitmap->load_bits(i, bits);
        }

        // Data bitmap
        data_bitmap = new Bitmap(sb->cluster_count);
        ints = (sb->cluster_count + BITMAP_BITS - 1) / BITMAP_BITS;
        for (uint32_t i = 0; i < ints; i++){
            in_file.read(reinterpret_cast<char*>(&bits), sizeof(bits));
            data_bitmap->load_bits(i, bits);
        }

        // Inodes
        for (uint32_t i = 0; i < sb->inode_count; i++){
            auto *inode = new IndexNode(i);
            in_file.read(reinterpret_cast<char*>(inode), sizeof(IndexNode));
            inode_vector.emplace_back(inode);
        }

        // Data blocks
        for (uint32_t i = 0; i < sb->cluster_count; i++){
            std::array<unsigned char, CLUSTER_SIZE> data = {};
            data.fill('\0');
            in_file.read(reinterpret_cast<char*>(data.data()), data.size());
            data_vector.emplace_back(data);
        }

        // Load root
        curr_dir = new Directory(data_vector[0]);
        in_file.close();
    } else {
        initialized = false;
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
    }
    out_file.open(fs_name.c_str(), std::ios::binary);
}

FileSystem::~FileSystem(){
    std::cout << "Saving to file" << std::endl;
    write_all();
    out_file.close();

    delete sb;
    delete inode_bitmap;
    delete data_bitmap;
    delete curr_dir;
    for (auto *inode : inode_vector) delete inode;
}

bool FileSystem::is_initialized() const{
    return initialized;
}

std::string FileSystem::get_name() const{
    return fs_name;
}

void FileSystem::free_inode(uint32_t id){
    uint32_t dir_data = inode_vector[id]->direct1;
    std::array<unsigned char, CLUSTER_SIZE> data = {};
    data.fill('\0');
    data_bitmap->set_free(dir_data);
    data_vector[dir_data] = data;

    inode_bitmap->set_free(id);
    inode_vector[id]->reset();
}

uint32_t FileSystem::get_directory_data(const std::string &path){
    // Parse path
    std::vector<std::string> tokens;
    std::istringstream iss(path);
    std::string token;
    while (std::getline(iss, token, '/')){
        if (!token.empty()) tokens.emplace_back(token);
    }

    // Path is absolute or relavite
    Directory *temp;
    uint32_t data_temp, inode_next;
    if (path[0] == '/') data_temp = 0;
    else data_temp = inode_vector[curr_dir->self]->direct1;

    // Dive into subdirectories
    for (std::string &part : tokens){
        temp = new Directory(data_vector[data_temp]);
        inode_next = temp->get_file_inode(part);
        delete temp;
        if (inode_next == INVALID){
            std::cout << "Path not found" << std::endl;
            return INVALID;
        }
        if (!inode_vector[inode_next]->is_directory){
            std::cout << "Not a directory" << std::endl;
            return INVALID;
        }
        data_temp = inode_vector[inode_next]->direct1;
    }
    return data_temp;
}

std::array<std::string, 2> FileSystem::parse_path_and_name(std::string &path){
    std::array<std::string, 2> duo;

    // Remove redundant slashes at end
    std::size_t last_char = path.find_last_not_of('/');
    if (last_char != std::string::npos){
        path = path.substr(0, last_char + 1);
    }

    // Separate name from path
    std::string raw_name, raw_path;
    size_t slash = path.find_last_of('/');
    if (slash != std::string::npos){
        if (slash == 0 && path.length() > 1) raw_path = path.substr(0, slash + 1);
        else raw_path = path.substr(0, slash);
        raw_name = path.substr(slash + 1, path.length());
    } else {
        raw_name = path;
    }

    duo[0] = raw_path;
    duo[1] = raw_name;
    return duo;
}

void FileSystem::cp(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::mv(std::string &source, std::string &dest){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get parsed path and name
    std::array<std::string, 2> source_param = parse_path_and_name(source);
    std::array<std::string, 2> dest_param = parse_path_and_name(dest);

    // Get parent data block
    uint32_t data_parent_source;
    if (!source_param[0].empty()){
        data_parent_source = get_directory_data(source_param[0]);
        if (data_parent_source == INVALID) return;
    } else data_parent_source = inode_vector[curr_dir->self]->direct1;

    uint32_t data_parent_dest;
    if (!dest_param[0].empty()){
        data_parent_dest = get_directory_data(dest_param[0]);
        if (data_parent_dest == INVALID) return;
    } else data_parent_dest = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (source_param[1].empty() || dest_param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing source file, already existing destination file
    auto *source_parent = new Directory(data_vector[data_parent_source]);
    uint32_t to_move = source_parent->get_file_inode(source_param[1]);

    auto *dest_parent = new Directory(data_vector[data_parent_dest]);
    uint32_t to_replace = dest_parent->get_file_inode(dest_param[1]);

    if (to_move == INVALID || to_replace != INVALID){
        std::cout << "Cannot move file" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Source is directory
    if (inode_vector[to_move]->is_directory){
        std::cout << "Source not file" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Update parent directories
    uint32_t file_inode = source_parent->get_file_inode(source_param[1]);
    source_parent->remove_file(source_param[1]);
    uint32_t source_parent_data = inode_vector[source_parent->self]->direct1;
    data_vector[source_parent_data] = source_parent->serialize();

    dest_parent->add_file(dest_param[1], file_inode);
    uint32_t dest_parent_data = inode_vector[dest_parent->self]->direct1;
    if (source_parent_data == dest_parent_data) {
        dest_parent->remove_file(source_param[1]);
    }
    data_vector[dest_parent_data] = dest_parent->serialize();


    // Update current directory
    if (source_parent->self == curr_dir->self){
        delete curr_dir;
        curr_dir = new Directory(data_vector[source_parent_data]);
    }
    if (dest_parent->self == curr_dir->self){
        delete curr_dir;
        curr_dir = new Directory(data_vector[dest_parent_data]);
    }
    delete source_parent;
    delete dest_parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::rm(const std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::mkdir(std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(dir_name);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "Path not found" << std::endl;
        return;
    }

    // Existing name
    auto *parent = new Directory(data_vector[data_parent]);
    if (parent->get_file_inode(param[1]) != INVALID){
        std::cout << "Exist" << std::endl;
        delete parent;
        return;
    }

    // Assign inode
    uint32_t inode_id = inode_bitmap->get_free();
    if (inode_id == INVALID){
        std::cout << "Cannot create directory - out of inodes" << std::endl;
        delete parent;
        return;
    }

    // Assign data block
    uint32_t data_id = data_bitmap->get_free();
    if (data_id == INVALID){
        std::cout << "Cannot create directory - out of data blocks" << std::endl;
        inode_bitmap->set_free(inode_id);
        delete parent;
        return;
    }

    // Write new directory to data block
    auto *new_dir = new Directory(inode_id, parent->self);
    inode_vector[inode_id]->set_directory(data_id);
    data_vector[data_id] = new_dir->serialize();

    // Update parent directory
    uint32_t parent_data = inode_vector[parent->self]->direct1;
    parent->add_file(param[1], inode_id);
    data_vector[parent_data] = parent->serialize();

    // Update current directory
    if (parent->self == curr_dir->self){
        delete curr_dir;
        curr_dir = new Directory(data_vector[parent_data]);
    }
    delete parent;
    delete new_dir;

    std::cout << "Ok" << std::endl;
}

void FileSystem::rmdir(std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(dir_name);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "Path not found" << std::endl;
        return;
    }

    // Non-existing name
    auto *parent = new Directory(data_vector[data_parent]);
    uint32_t to_remove = parent->get_file_inode(param[1]);
    if (to_remove == INVALID) {
        std::cout << "File not found" << std::endl;
        delete parent;
        return;
    }

    // Directory contains some files
    auto *dir = new Directory(data_vector[inode_vector[to_remove]->direct1]);
    if (!dir->is_empty()) {
        std::cout << "Cannot remove directory - not empty" << std::endl;
        delete parent;
        delete dir;
        return;
    }

    // Update current directory
    if (curr_dir->self == to_remove){
        delete curr_dir;
        curr_dir = new Directory(data_vector[0]);
    }

    // Update parent directory
    parent->remove_file(param[1]);
    free_inode(to_remove);
    uint32_t parent_data = inode_vector[parent->self]->direct1;
    data_vector[parent_data] = parent->serialize();

    // Update current directory
    if (parent->self == curr_dir->self){
        delete curr_dir;
        curr_dir = new Directory(data_vector[parent_data]);
    }
    delete parent;
    delete dir;

    std::cout << "Ok" << std::endl;
}

void FileSystem::ls(const std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Empty parameter, print current directory content
    if (dir_name.empty()){
        for (auto file : curr_dir->content){
            if (inode_vector[file->inode]->is_directory) std::cout << "+" << file->name << std::endl;
            else std::cout << "-" << file->name << std::endl;
        }
        return;
    }

    // Get relevant data block
    uint32_t data_temp = get_directory_data(dir_name);
    if (data_temp == INVALID) return;

    // Print content
    auto *temp = new Directory(data_vector[data_temp]);
    for (auto file : temp->content){
        if (inode_vector[file->inode]->is_directory) std::cout << "+" << file->name << std::endl;
        else std::cout << "-" << file->name << std::endl;
    }
    delete temp;
}

void FileSystem::cat(const std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::cd(const std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get relevant data block
    uint32_t data_temp = get_directory_data(dir_name);
    if (data_temp == INVALID) return;

    // Set new current directory
    delete curr_dir;
    curr_dir = new Directory(data_vector[data_temp]);

    std::cout << "Ok" << std::endl;
}

void FileSystem::pwd(){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Build path from curr_dir
    std::vector<std::string> path;
    uint32_t id_self = curr_dir->self;
    uint32_t id_parent = curr_dir->parent;
    while(id_self != 0){
        uint32_t id_data = inode_vector[id_parent]->direct1;
        auto *dir = new Directory(data_vector[id_data]);
        path.emplace_back(dir->get_file_name(id_self));

        id_self = dir->self;
        id_parent = dir->parent;
        delete dir;
    }

    // Output path
    if (path.empty()){
        std::cout << "/" << std::endl;
    } else {
        std::stringstream ss;
        for (auto it = path.rbegin(); it != path.rend(); it++){
            std::cout << "/" << *it;
        }
        std::cout << ss.str() << std::endl;
    }
}

void FileSystem::info(std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(file);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing name
    auto *parent = new Directory(data_vector[data_parent]);
    uint32_t id = parent->get_file_inode(param[1]);
    if (id == INVALID) {
        std::cout << "File not found" << std::endl;
        return;
    }

    // Print content
    std::stringstream ss;
    auto *file_inode = inode_vector[id];
    ss << param[1] << " - " << file_inode->file_size << " - i-node " << id << " - " << file_inode->occupied_blocks();
    std::cout << ss.str() << std::endl;

    delete parent;
}

void FileSystem::incp(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::outcp(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::ln(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::format(uint32_t size){
    if (size < MINIMUM_FORMAT_SIZE){
        std::cout << "Cannot create file" << std::endl;
        return;
    }

    delete sb;
    delete inode_bitmap;
    delete data_bitmap;
    delete curr_dir;
    for (auto *inode : inode_vector) delete inode;

    inode_vector.clear();
    data_vector.clear();

    sb = new Superblock();
    sb->init(size);
    inode_bitmap = new Bitmap(sb->inode_count);
    data_bitmap = new Bitmap(sb->cluster_count);

    for (uint32_t i = 0; i < sb->inode_count; i++){
        inode_vector.emplace_back(new IndexNode(i));
    }

    for (uint32_t i = 0; i < sb->cluster_count; i++){
        std::array<unsigned char, CLUSTER_SIZE> data = {};
        data.fill('\0');

        std::string sourceString = "Hello from data block ";
        sourceString += std::to_string(i);
        std::copy(std::begin(sourceString), std::end(sourceString), data.begin());

        data_vector.emplace_back(data);
    }

    // Setup root dir
    uint32_t inode_id = inode_bitmap->get_free();
    auto *root_node = inode_vector[inode_id];
    curr_dir = new Directory(inode_id, inode_id);

    uint32_t data_id = data_bitmap->get_free();
    root_node->set_directory(data_id);
    data_vector[data_id] = curr_dir->serialize();

    initialized = true;
    write_all();

    std::cout << "Ok" << std::endl;
}

////////////////////////////////////

void FileSystem::write_all(){
    uint32_t ints;

    if (!out_file.is_open()){
        out_file.open(fs_name.c_str(), std::ios::binary);
    }
    out_file.seekp(0, std::ios::beg);

    // Superblock
    out_file.write(reinterpret_cast<const char*>(sb), sizeof(Superblock));

    // Inode bitmap
    ints = (sb->inode_count + BITMAP_BITS - 1) / BITMAP_BITS;
    for(int i = 0; i < ints; i++){
        uint32_t bits = inode_bitmap->get_bits(i);
        out_file.write(reinterpret_cast<const char*>(&bits), sizeof(bits));
    }

    // Data bitmap
    ints = (sb->cluster_count + BITMAP_BITS - 1) / BITMAP_BITS;
    for(uint32_t i = 0; i < ints; i++){
        uint32_t bits = data_bitmap->get_bits(i);
        out_file.write(reinterpret_cast<const char*>(&bits), sizeof(bits));
    }

    // Inodes
    for (uint32_t i = 0; i < sb->inode_count; i++){
        auto *inode = inode_vector[i];
        out_file.write(reinterpret_cast<const char*>(inode), sizeof(IndexNode));
    }

    // Data blocks
    for (uint32_t i = 0; i < sb->cluster_count; i++){
        std::array<unsigned char, CLUSTER_SIZE> data = data_vector[i];
        out_file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
}