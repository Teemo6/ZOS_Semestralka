#include <utility>
#include <iostream>
#include <sstream>

#include "FileSystem.hpp"
#include "IndexNode.hpp"
#include "DirectoryItem.hpp"

FileSystem::FileSystem(std::string new_name){
    fs_name = std::move(new_name);

    sb = new Superblock();
    inode_bitmap = nullptr;
    data_bitmap = nullptr;
    curr_dir = nullptr;
    inode_vector = std::vector<IndexNode *>();
    data_vector = std::vector<std::array<unsigned char, CLUSTER_SIZE>>();

    std::ifstream in_file(fs_name.c_str(), std::ios::binary);
    if (in_file.is_open()){
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

uint32_t FileSystem::get_dir_from_path(const std::string &path){
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

void FileSystem::cp(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::mv(const std::string &file1, const std::string &file2){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::rm(const std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::mkdir(const std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    uint32_t inode_id = inode_bitmap->get_free();
    if (inode_id == INVALID){
        std::cout << "Cannot create directory - out of inodes" << std::endl;
        return;
    }

    uint32_t data_id = data_bitmap->get_free();
    if (data_id == INVALID){
        std::cout << "Cannot create directory - out of data blocks" << std::endl;
        inode_bitmap->set_free(inode_id);
        return;
    }

    // TODO fs_name check

    // Create new directory
    auto *new_dir = new Directory(inode_id, curr_dir->self);
    inode_vector[inode_id]->set_directory(data_id);
    data_vector[data_id] = new_dir->serialize();

    // Update parent directory
    uint32_t curr_data = inode_vector[curr_dir->self]->direct1;
    curr_dir->add_file(dir_name, inode_id);
    data_vector[curr_data] = curr_dir->serialize();

    std::cout << "Ok" << std::endl;
}

void FileSystem::rmdir(const std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    uint32_t id;

    // TODO absolute
    if (dir_name[0] == '/'){
        std::cout << "ABSOLUTE" << std::endl;
        return;
    } else {
        id = curr_dir->get_file_inode(dir_name);
    }

    if (id == INVALID){
        std::cout << "File not found" << std::endl;
        return;
    }

    // TODO non empty

    if (curr_dir->remove_file(dir_name)){
        free_inode(id);
        uint32_t curr_data = inode_vector[curr_dir->self]->direct1;
        data_vector[curr_data] = curr_dir->serialize();
    } else {
        std::cout << "File not found" << std::endl;
    }
}

// DONE
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
    uint32_t data_temp = get_dir_from_path(dir_name);
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


// DONE
void FileSystem::cd(const std::string &dir_name){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

    // Get relevant data block
    uint32_t data_temp = get_dir_from_path(dir_name);
    if (data_temp == INVALID) return;

    // Set new current directory
    delete curr_dir;
    curr_dir = new Directory(data_vector[data_temp]);

    std::cout << "Ok" << std::endl;
}

// DONE
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

void FileSystem::info(const std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

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

void FileSystem::load(const std::string &file){
    if (!initialized){
        std::cout << CALL_FORMAT_MESSAGE << std::endl;
        return;
    }

}

void FileSystem::format(uint32_t size){
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
        auto *inode = new IndexNode(i);
        inode_vector.emplace_back(inode);
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