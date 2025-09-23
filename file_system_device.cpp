#include "file_system_device.h"
#include <iostream>
#include <fstream>

void FileSystemDevice::createFile(const std::string& parent_path, const std::string& file_name, const std::vector<std::string>& file_content) {
    // Find parent directory (simplified for example)
    Directory* parent = findDirectory(parent_path);
    if (parent) {
        parent->files.push_back(std::make_unique<FileEntry>(file_name, file_content));
        std::cout << "Created file: " << parent_path << "/" << file_name << std::endl;
    } else {
        std::cerr << "Error: Parent directory not found: " << parent_path << std::endl;
    }
}

void FileSystemDevice::listContents(const std::string& path) {
    Directory* dir = findDirectory(path);
    if (dir) {
        std::cout << "Contents of " << path << ":" << std::endl;
        for (const auto& file : dir->files) {
            std::cout << "  File: " << file->name << " (Size: " << file->size << " bytes)" << std::endl;
        }
        for (const auto& sub_dir : dir->subdirectories) {
            std::cout << "  Directory: " << sub_dir->name << std::endl;
        }
    } else {
        std::cerr << "Error: Directory not found: " << path << std::endl;
    }
}

void FileSystemDevice::save() {
    std::ofstream os(persistence_file);
    cereal::JSONOutputArchive archive(os);
    archive(CEREAL_NVP(root_directory));
    std::cout << "File system saved to " << persistence_file << std::endl;
}

void FileSystemDevice::load() {
    std::ifstream is(persistence_file);
    if (is.is_open()) {
        cereal::JSONInputArchive archive(is);
        archive(CEREAL_NVP(root_directory));
        std::cout << "File system loaded from " << persistence_file << std::endl;
    } else {
        std::cout << "No existing file system found. Starting with empty root." << std::endl;
        root_directory = std::make_unique<Directory>("root");
    }
}

Directory* FileSystemDevice::findDirectory(const std::string& path) {
    if (path == "/root" || path == "/") { // Simple check for root
        return root_directory.get();
    }
    // Basic example: only finds direct subdirectories of root for now
    // A full implementation would parse the path and traverse the tree
    if (path.rfind("/root/", 0) == 0 && path.length() > 6) {
        std::string dir_name = path.substr(6);
        for (const auto& sub_dir : root_directory->subdirectories) {
            if (sub_dir->name == dir_name) {
                return sub_dir.get();
            }
        }
    }
    return nullptr;
}
