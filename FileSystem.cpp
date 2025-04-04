#include <iostream>
#include <sstream>

#include "FileSystem.hpp"

FileSystem::FileSystem(std::string new_name) : fs_name(std::move(new_name)){
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
            // std::memset(inode, 0, sizeof(IndexNode));
            in_file.read(reinterpret_cast<char*>(inode), sizeof(IndexNode));
            inode_vector.emplace_back(inode);
        }

        // Data blocks
        for (uint32_t i = 0; i < sb->cluster_count; i++){
            std::array<unsigned char, CLUSTER_SIZE> data = {};
            in_file.read(reinterpret_cast<char*>(data.data()), data.size());
            data_vector.emplace_back(data);
        }
        in_file.close();

        // Load root
        if (!data_vector.empty()){
            curr_dir = new Directory(data_vector[0]);
        } else {
            initialized = false;
            std::cout << CALL_FORMAT_MESSAGE << std::endl;
        }
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

void FileSystem::cp(std::string &source, std::string &dest){
    // Get parsed path and name
    std::array<std::string, 2> source_param = parse_path_and_name(source);
    std::array<std::string, 2> dest_param = parse_path_and_name(dest);

    // Get parent data block
    uint32_t data_parent_source;
    if (!source_param[0].empty()){
        data_parent_source = get_directory_data_block(source_param[0]);
        if (data_parent_source == INVALID) return;
    } else data_parent_source = inode_vector[curr_dir->self]->direct1;

    uint32_t data_parent_dest;
    if (!dest_param[0].empty()){
        data_parent_dest = get_directory_data_block(dest_param[0]);
        if (data_parent_dest == INVALID) return;
    } else data_parent_dest = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (source_param[1].empty() || dest_param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing source file, already existing destination file
    auto *source_parent = new Directory(data_vector[data_parent_source]);
    uint32_t id_source = source_parent->get_file_inode(source_param[1]);

    auto *dest_parent = new Directory(data_vector[data_parent_dest]);
    uint32_t id_dest = dest_parent->get_file_inode(dest_param[1]);

    if (id_source == INVALID || id_dest != INVALID){
        std::cout << "File not found" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Source is directory
    auto *source_inode = inode_vector[id_source];
    if (source_inode->is_directory){
        std::cout << "Cannot make copy of directory" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Assign inode
    uint32_t inode_id = inode_bitmap->get_free();
    if (inode_id == INVALID){
        std::cout << "Out of free inodes" << std::endl;
        return;
    }

    // Determine blocks needed
    if (!data_bitmap->check_free(get_file_data_blocks(id_source).size())){
        std::cout << "Out of free data blocks" << std::endl;
        inode_bitmap->set_free(inode_id);
        return;
    }

    // Update inode metadata
    auto *dest_inode = inode_vector[inode_id];
    dest_inode->raw_size = source_inode->raw_size;

    // Copy direct blocks to inode
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> content = get_file_content(id_source);
    for (int i = 0; i < DIRECT_REF; i++){
        if (content.empty()) break;
        uint32_t data_id = data_bitmap->get_free();
        dest_inode->write_direct_data(data_id);
        data_vector[data_id] = content.front();
        content.pop_front();
    }

    // Copy indirect 1 blocks to inode
    if (!content.empty()){
        auto *indir_1 = new ReferenceBlock();
        for (int i = 0; i < INDIRECT_1_REF; i++){
            if (content.empty()) break;
            uint32_t data_id = data_bitmap->get_free();
            indir_1->add_reference(data_id);
            data_vector[data_id] = content.front();
            content.pop_front();
        }
        uint32_t data_id = data_bitmap->get_free();
        dest_inode->indirect1 = data_id;
        data_vector[data_id] = indir_1->serialize();
        delete indir_1;
    }

    // Copy indirect 2 blocks to inode
    if (!content.empty()){
        auto *indir_2 = new ReferenceBlock();
        for (int i = 0; i < INDIRECT_2_REF; i++){
            if (content.empty()) break;
            auto *indir_1 = new ReferenceBlock();
            for (int j = 0; j < INDIRECT_1_REF; j++){
                if (content.empty()) break;
                uint32_t data_id = data_bitmap->get_free();
                indir_1->add_reference(data_id);
                data_vector[data_id] = content.front();
                content.pop_front();
            }
            uint32_t data_id = data_bitmap->get_free();
            indir_2->add_reference(data_id);
            data_vector[data_id] = indir_1->serialize();
            delete indir_1;
        }
        uint32_t data_id = data_bitmap->get_free();
        dest_inode->indirect2 = data_id;
        data_vector[data_id] = indir_2->serialize();
        delete indir_2;
    }

    // Update dest directory
    dest_parent->add_file(dest_param[1], inode_id);
    uint32_t dest_parent_data = inode_vector[dest_parent->self]->direct1;
    data_vector[dest_parent_data] = dest_parent->serialize();

    update_curr_dir_if_same(dest_parent->self);
    delete source_parent;
    delete dest_parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::mv(std::string &source, std::string &dest){
    // Get parsed path and name
    std::array<std::string, 2> source_param = parse_path_and_name(source);
    std::array<std::string, 2> dest_param = parse_path_and_name(dest);

    // Get parent data block
    uint32_t data_parent_source;
    if (!source_param[0].empty()){
        data_parent_source = get_directory_data_block(source_param[0]);
        if (data_parent_source == INVALID) return;
    } else data_parent_source = inode_vector[curr_dir->self]->direct1;

    uint32_t data_parent_dest;
    if (!dest_param[0].empty()){
        data_parent_dest = get_directory_data_block(dest_param[0]);
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

    if (to_move == INVALID){
        std::cout << "File not found" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Called with no specified name
    if (to_replace != INVALID){
        if (inode_vector[to_replace]->is_directory){
            delete dest_parent;
            dest_parent = new Directory(data_vector[inode_vector[to_replace]->direct1]);
            dest_param[1] = source_param[1];
            to_replace = dest_parent->get_file_inode( dest_param[1]);
            if (to_replace != INVALID){
                std::cout << "Exist" << std::endl;
                delete source_parent;
                delete dest_parent;
                return;
            }
        } else {
            std::cout << "Path not found" << std::endl;
            delete source_parent;
            delete dest_parent;
            return;
        }
    }

    // Update source and dest directories
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

    // Update moved directory parent
    if (inode_vector[to_move]->is_directory){
        uint32_t source_dir_data = inode_vector[to_move]->direct1;
        auto *source_dir = new Directory(data_vector[source_dir_data]);
        source_dir->update_parent(dest_parent->self);
        data_vector[source_dir_data] = source_dir->serialize();
        delete source_dir;
    }

    // Update current directory
    update_curr_dir_if_same(source_parent->self);
    update_curr_dir_if_same(dest_parent->self);
    delete source_parent;
    delete dest_parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::rm(std::string &file){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(file);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing name
    auto *parent = new Directory(data_vector[data_parent]);
    uint32_t file_inode = parent->get_file_inode(param[1]);
    if (file_inode == INVALID){
        std::cout << "File not found" << std::endl;
        delete parent;
        return;
    }

    // Call to directory
    if (inode_vector[file_inode]->is_directory){
        std::cout << "Cannot 'rm' directory, call 'rmdir'" << std::endl;
        delete parent;
        return;
    }

    // Empty data blocks if inode has no other references
    if (inode_vector[file_inode]->references == 1){
        uint32_t size = CLUSTER_SIZE;
        std::deque<uint32_t> data_blocks = get_file_data_blocks(file_inode);
        while (!data_blocks.empty()){
            uint32_t id = data_blocks.front();
            data_bitmap->set_free(id);
            data_vector[id] = create_empty_data_block(id);
            data_blocks.pop_front();
        }

        // Free inode
        inode_bitmap->set_free(file_inode);
        inode_vector[file_inode]->reset();
    } else {
        inode_vector[file_inode]->references--;
    }

    // Update parent directory
    parent->remove_file(param[1]);
    uint32_t parent_data = inode_vector[parent->self]->direct1;
    data_vector[parent_data] = parent->serialize();

    // Update current directory
    update_curr_dir_if_same(parent->self);
    delete parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::mkdir(std::string &dir_name){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(dir_name);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
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

    // Directory full
    if (parent->content.size() >= CLUSTER_SIZE / sizeof(DirectoryItem)){
        std::cout << "Cannot create more directories" << std::endl;
        delete parent;
        return;
    }

    // Assign inode
    uint32_t inode_id = inode_bitmap->get_free();
    if (inode_id == INVALID){
        std::cout << "Out of free inodes" << std::endl;
        delete parent;
        return;
    }

    // Assign data block
    uint32_t data_id = data_bitmap->get_free();
    if (data_id == INVALID){
        std::cout << "Out of free data blocks" << std::endl;
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
    update_curr_dir_if_same(parent->self);
    delete parent;
    delete new_dir;

    std::cout << "Ok" << std::endl;
}

void FileSystem::rmdir(std::string &dir_name){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(dir_name);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
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
    free_directory_inode(to_remove);
    uint32_t parent_data = inode_vector[parent->self]->direct1;
    data_vector[parent_data] = parent->serialize();

    // Update current directory
    update_curr_dir_if_same(parent->self);
    delete parent;
    delete dir;

    std::cout << "Ok" << std::endl;
}

void FileSystem::ls(const std::string &dir_name){
    // Empty parameter, print current directory content
    if (dir_name.empty()){
        for (auto file : curr_dir->content){
            if (inode_vector[file->inode]->is_directory) std::cout << "+" << file->name << std::endl;
            else std::cout << "-" << file->name << std::endl;
        }
        return;
    }

    // Get relevant data block
    uint32_t data_temp = get_directory_data_block(dir_name);
    if (data_temp == INVALID) return;

    // Print content
    auto *temp = new Directory(data_vector[data_temp]);
    for (auto file : temp->content){
        if (inode_vector[file->inode]->is_directory) std::cout << "+" << file->name << std::endl;
        else std::cout << "-" << file->name << std::endl;
    }
    delete temp;
}

void FileSystem::cat(std::string &file){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(file);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "Path not found" << std::endl;
        return;
    }

    // Existing name
    auto *parent = new Directory(data_vector[data_parent]);
    uint32_t file_inode = parent->get_file_inode(param[1]);
    if (file_inode == INVALID){
        std::cout << "File not found" << std::endl;
        delete parent;
        return;
    }

    // Call to directory
    if (inode_vector[file_inode]->is_directory){
        std::cout << "Cannot 'cat' directory" << std::endl;
        delete parent;
        return;
    }

    // Print file content
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> content = get_file_content(file_inode);
    std::stringstream ss;
    while (!content.empty()){
        for (int i = 0; i < CLUSTER_SIZE; i++){
            ss << content.front()[i];
        }
        content.pop_front();
    }
    delete parent;

    std::cout << ss.str() << std::endl;
}

void FileSystem::cd(const std::string &dir_name){
    // Get relevant data block
    uint32_t data_temp = get_directory_data_block(dir_name);
    if (data_temp == INVALID) return;

    // Set new current directory
    delete curr_dir;
    curr_dir = new Directory(data_vector[data_temp]);

    std::cout << "Ok" << std::endl;
}

void FileSystem::pwd(){
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
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(file);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
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

    // Count indirect blocks
    uint32_t in1_d = 0, in1_i = 0, in2_d = 0, in2_i = 0, in2_i2 = 0;
    auto *info_inode = inode_vector[id];
    if (info_inode->indirect1 != INVALID){
        auto *ref = new ReferenceBlock(data_vector[info_inode->indirect1]);
        in1_d = ref->get_reference_count();
        in1_i = 1;
        delete ref;
    }
    if (info_inode->indirect2 != INVALID){
        auto *ref = new ReferenceBlock(data_vector[info_inode->indirect2]);
        for (uint32_t i = 0; i < ref->get_reference_count(); i++){
            auto *ref_1 = new ReferenceBlock(data_vector[ref->get_reference(i)]);
            in2_d += ref_1->get_reference_count();
            delete ref_1;
        }
        in2_i = ref->get_reference_count();
        in2_i2 = 1;
        delete ref;
    }

    // Print content
    std::stringstream ss;
    std::string name = parent->get_file_name(id);
    if (id == 0) name = "/";
    ss << name
       << " - file size " << get_file_data_blocks(id).size() * CLUSTER_SIZE
       << " - i-node " << id
       << " - direct blocks " << info_inode->occupied_blocks()
       << " - indirect 1 blocks " << in1_d << "+" << in1_i
       << " - indirect 2 blocks " << in2_d << "+" << in2_i << "+" << in2_i2;
    delete parent;

    std::cout << ss.str() << std::endl;
}

void FileSystem::incp(const std::string &system, std::string &virt){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(virt);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
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

    // Try open file
    std::ifstream file(system);
    if (!file.is_open()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Determine blocks needed
    file.seekg(0, std::ios::end);
    uint32_t file_size = file.tellg();
    uint32_t needed = (file_size + CLUSTER_SIZE) / CLUSTER_SIZE;
    file.seekg(0, std::ios::beg);
    if (needed > DIRECT_REF){
        if (needed > DIRECT_REF + INDIRECT_1_REF){
            needed += (needed + INDIRECT_1_REF) / INDIRECT_1_REF;
            needed++;
        }
        needed++;
    }

    // Check free data blocks
    if (!data_bitmap->check_free(needed) || needed > MAX_FILE_SIZE){
        std::cout << "Out of free data blocks" << std::endl;
        file.close();
        return;
    }

    // Directory full
    if (parent->content.size() >= CLUSTER_SIZE / sizeof(DirectoryItem)){
        std::cout << "Cannot create more files" << std::endl;
        file.close();
        delete parent;
        return;
    }

    // Assign inode
    uint32_t inode_id = inode_bitmap->get_free();
    if (inode_id == INVALID){
        std::cout << "Out of free inodes" << std::endl;
        return;
    }

    // Read all data
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> all_data;
    while (!file.eof()) {
        std::array<unsigned char, CLUSTER_SIZE> data = {};
        file.read(reinterpret_cast<char*>(data.data()), CLUSTER_SIZE);
        if (file.gcount() > 0) all_data.push_back(data);
    }

    // Update inode metadata
    auto *file_inode = inode_vector[inode_id];
    file_inode->references = 1;
    file_inode->raw_size = file_size;

    // Write direct blocks to inode
    for (int i = 0; i < DIRECT_REF; i++){
        if (all_data.empty()) break;
        uint32_t data_id = data_bitmap->get_free();
        file_inode->write_direct_data(data_id);
        data_vector[data_id] = all_data.front();
        all_data.pop_front();
    }

    // Write indirect 1 blocks to inode
    if (!all_data.empty()){
        auto *indir_1 = new ReferenceBlock();
        for (int i = 0; i < INDIRECT_1_REF; i++){
            if (all_data.empty()) break;
            uint32_t data_id = data_bitmap->get_free();
            indir_1->add_reference(data_id);
            data_vector[data_id] = all_data.front();
            all_data.pop_front();
        }
        uint32_t data_id = data_bitmap->get_free();
        file_inode->indirect1 = data_id;
        data_vector[data_id] = indir_1->serialize();
        delete indir_1;
    }

    // Write indirect 2 blocks to inode
    if (!all_data.empty()){
        auto *indir_2 = new ReferenceBlock();
        for (int i = 0; i < INDIRECT_2_REF; i++){
            if (all_data.empty()) break;
            auto *indir_1 = new ReferenceBlock();
            for (int j = 0; j < INDIRECT_1_REF; j++){
                if (all_data.empty()) break;
                uint32_t data_id = data_bitmap->get_free();
                indir_1->add_reference(data_id);
                data_vector[data_id] = all_data.front();
                all_data.pop_front();
            }
            uint32_t data_id = data_bitmap->get_free();
            indir_2->add_reference(data_id);
            data_vector[data_id] = indir_1->serialize();
            delete indir_1;
        }
        uint32_t data_id = data_bitmap->get_free();
        file_inode->indirect2 = data_id;
        data_vector[data_id] = indir_2->serialize();
        delete indir_2;
    }

    // Update parent directory
    uint32_t parent_data = inode_vector[parent->self]->direct1;
    parent->add_file(param[1], inode_id);
    data_vector[parent_data] = parent->serialize();

    // Update current directory
    update_curr_dir_if_same(parent->self);
    delete parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::outcp(std::string &virt, const std::string &system){
    // Get parsed path and name
    std::array<std::string, 2> param = parse_path_and_name(virt);

    // Get parent data block
    uint32_t data_parent;
    if (!param[0].empty()) {
        data_parent = get_directory_data_block(param[0]);
        if (data_parent == INVALID) return;
    } else data_parent = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing name
    auto *parent = new Directory(data_vector[data_parent]);
    uint32_t file_inode = parent->get_file_inode(param[1]);
    if (file_inode == INVALID){
        std::cout << "File not found" << std::endl;
        delete parent;
        return;
    }

    // Try open file
    std::ofstream file(system);
    if (!file.is_open()){
        std::cout << "Could not create file" << std::endl;
        return;
    }
    file.seekp(0, std::ios::beg);

    // Write data to file
    uint32_t size = CLUSTER_SIZE;
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> content = get_file_content(file_inode);
    while (!content.empty()){
        if (content.size() == 1){
            size = inode_vector[file_inode]->raw_size % CLUSTER_SIZE;
            if (size == 0) size = CLUSTER_SIZE;
        }
        file.write(reinterpret_cast<const char*>(content.front().data()), size);
        content.pop_front();
    }
    file.close();
    delete parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::ln(std::string &source, std::string &link){
    // Get parsed path and name
    std::array<std::string, 2> source_param = parse_path_and_name(source);
    std::array<std::string, 2> dest_param = parse_path_and_name(link);

    // Get parent data block
    uint32_t data_parent_source;
    if (!source_param[0].empty()){
        data_parent_source = get_directory_data_block(source_param[0]);
        if (data_parent_source == INVALID) return;
    } else data_parent_source = inode_vector[curr_dir->self]->direct1;

    uint32_t data_parent_dest;
    if (!dest_param[0].empty()){
        data_parent_dest = get_directory_data_block(dest_param[0]);
        if (data_parent_dest == INVALID) return;
    } else data_parent_dest = inode_vector[curr_dir->self]->direct1;

    // Empty name
    if (source_param[1].empty() || dest_param[1].empty()){
        std::cout << "File not found" << std::endl;
        return;
    }

    // Non-existing source file, already existing destination file
    auto *source_parent = new Directory(data_vector[data_parent_source]);
    uint32_t id_source = source_parent->get_file_inode(source_param[1]);

    auto *dest_parent = new Directory(data_vector[data_parent_dest]);
    uint32_t id_dest = dest_parent->get_file_inode(dest_param[1]);

    if (id_source == INVALID || id_dest != INVALID){
        std::cout << "File not found" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Hardlink to directory
    auto *source_inode = inode_vector[id_source];
    if (source_inode->is_directory){
        std::cout << "Cannot 'ln' directory" << std::endl;
        delete source_parent;
        delete dest_parent;
        return;
    }

    // Update inode metadata
    source_inode->references++;

    // Update dest directory
    dest_parent->add_file(dest_param[1], id_source);
    uint32_t dest_parent_data = inode_vector[dest_parent->self]->direct1;
    data_vector[dest_parent_data] = dest_parent->serialize();

    update_curr_dir_if_same(dest_parent->self);
    delete source_parent;
    delete dest_parent;

    std::cout << "Ok" << std::endl;
}

void FileSystem::format(uint32_t size){
    if (size < MINIMUM_FORMAT_SIZE){
        std::cout << "Cannot create file, allocate at least " << MINIMUM_FORMAT_SIZE << " bytes" << std::endl;
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
        data_vector.emplace_back(create_empty_data_block(i));
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


void FileSystem::update_curr_dir_if_same(uint32_t id){
    if (id == curr_dir->self){
        delete curr_dir;
        curr_dir = new Directory(data_vector[inode_vector[id]->direct1]);
    }
}

void FileSystem::free_directory_inode(uint32_t id){
    uint32_t dir_data = inode_vector[id]->direct1;
    data_bitmap->set_free(dir_data);
    data_vector[dir_data] = create_empty_data_block(dir_data);

    inode_bitmap->set_free(id);
    inode_vector[id]->reset();
}

std::array<unsigned char, CLUSTER_SIZE> FileSystem::create_empty_data_block(uint32_t id){
    std::array<unsigned char, CLUSTER_SIZE> data = {};
    data.fill('\0');

    std::string sourceString = "Empty data block ";
    sourceString += std::to_string(id);
    std::copy(std::begin(sourceString), std::end(sourceString), data.begin());

    return data;
}

std::deque<uint32_t> FileSystem::get_file_data_blocks(uint32_t inode){
    std::deque<uint32_t> data_blocks;
    auto *file = inode_vector[inode];

    // Direct block
    if (file->direct1 != INVALID) data_blocks.push_back(file->direct1);
    if (file->direct2 != INVALID) data_blocks.push_back(file->direct2);
    if (file->direct3 != INVALID) data_blocks.push_back(file->direct3);
    if (file->direct4 != INVALID) data_blocks.push_back(file->direct4);
    if (file->direct5 != INVALID) data_blocks.push_back(file->direct5);

    // Indirect 1 blocks
    if (file->indirect1 != INVALID){
        data_blocks.push_back(file->indirect1);
        auto *ref = new ReferenceBlock(data_vector[file->indirect1]);
        for (int i = 0; i < ref->get_reference_count(); i++){
            data_blocks.push_back(ref->get_reference(i));
        }
        delete ref;
    }

    // Indirect 2 blocks
    if (file->indirect2 != INVALID){
        data_blocks.push_back(file->indirect2);
        auto *ref2 = new ReferenceBlock(data_vector[file->indirect2]);
        for (int i = 0; i < ref2->get_reference_count(); i++){
            data_blocks.push_back(ref2->get_reference(i));
            auto *ref1 = new ReferenceBlock(data_vector[ref2->get_reference(i)]);
            for (int j = 0; j < ref1->get_reference_count(); j++){
                data_blocks.push_back(ref1->get_reference(j));
            }
            delete ref1;
        }
        delete ref2;
    }
    return data_blocks;
}

std::deque<std::array<unsigned char, CLUSTER_SIZE>> FileSystem::get_file_content(uint32_t inode){
    std::deque<std::array<unsigned char, CLUSTER_SIZE>> all_data;
    auto *file = inode_vector[inode];

    // Direct block
    if (file->direct1 != INVALID) all_data.push_back(data_vector[file->direct1]);
    if (file->direct2 != INVALID) all_data.push_back(data_vector[file->direct2]);
    if (file->direct3 != INVALID) all_data.push_back(data_vector[file->direct3]);
    if (file->direct4 != INVALID) all_data.push_back(data_vector[file->direct4]);
    if (file->direct5 != INVALID) all_data.push_back(data_vector[file->direct5]);

    // Indirect 1 blocks
    if (file->indirect1 != INVALID){
        auto *ref = new ReferenceBlock(data_vector[file->indirect1]);
        for (int i = 0; i < ref->get_reference_count(); i++){
            all_data.push_back(data_vector[ref->get_reference(i)]);
        }
        delete ref;
    }

    // Indirect 2 blocks
    if (file->indirect2 != INVALID){
        auto *ref2 = new ReferenceBlock(data_vector[file->indirect2]);
        for (int i = 0; i < ref2->get_reference_count(); i++){
            auto *ref1 = new ReferenceBlock(data_vector[ref2->get_reference(i)]);
            for (int j = 0; j < ref1->get_reference_count(); j++){
                all_data.push_back(data_vector[ref1->get_reference(j)]);
            }
            delete ref1;
        }
        delete ref2;
    }
    return all_data;
}

uint32_t FileSystem::get_directory_data_block(const std::string &path){
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