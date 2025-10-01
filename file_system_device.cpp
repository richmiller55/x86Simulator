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

FileEntry* FileSystemDevice::findFile(const std::string& path) {
    // Simple implementation for "/root/filename.txt"
    size_t last_slash = path.find_last_of('/');
    if (last_slash == std::string::npos) return nullptr;

    std::string parent_path = path.substr(0, last_slash);
    if (parent_path.empty()) parent_path = "/";
    std::string file_name = path.substr(last_slash + 1);

    Directory* parent = findDirectory(parent_path);
    if (parent) {
        for (const auto& file : parent->files) {
            if (file->name == file_name) {
                return file.get();
            }
        }
    }
    return nullptr;
}

void FileSystemDevice::appendToFile(const std::string& file_path, char data) {
    FileEntry* file = findFile(file_path);
    
    if (!file) {
        size_t last_slash = file_path.find_last_of('/');
        std::string parent_path = (last_slash != std::string::npos) ? file_path.substr(0, last_slash) : "/";
        if (parent_path.empty()) parent_path = "/";
        std::string file_name = (last_slash != std::string::npos) ? file_path.substr(last_slash + 1) : file_path;
        
        Directory* parent = findDirectory(parent_path);
        if (parent) {
            parent->files.push_back(std::make_unique<FileEntry>(file_name));
            file = parent->files.back().get();
        } else {
            return; // Cannot find parent directory
        }
    }

    if (file->content.empty()) {
        file->content.push_back("");
    }

    if (data == '\n') {
        file->content.push_back("");
    } else {
        file->content.back() += data;
    }

    file->size = 0;
    for (const auto& line : file->content) {
        file->size += line.length() + 1; // +1 for newline
    }
}

const std::vector<std::string>* FileSystemDevice::getFileContent(const std::string& path) const {
    FileEntry* file = const_cast<FileSystemDevice*>(this)->findFile(path);
    if (file) {
        return &file->content;
    }
    return nullptr;
}
