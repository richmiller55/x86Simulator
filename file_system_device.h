#ifndef FILE_SYSTEM_DEVICE_H
#define FILE_SYSTEM_DEVICE_H

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <utility>

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

class FileEntry {
public:
    std::string name;
    std::vector<std::string> content;
    size_t size; // Can be derived from content.size() or total string length

    FileEntry(std::string n = "", std::vector<std::string> c = {}) :
        name(std::move(n)), content(std::move(c)), size(0) {
        for (const auto& line : content) {
            size += line.length(); // Simple size calculation
        }
    }

    template<class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(content), CEREAL_NVP(size));
    }
};

class Directory {
public:
    std::string name;
    std::vector<std::unique_ptr<FileEntry>> files;
    std::vector<std::unique_ptr<Directory>> subdirectories;

    Directory(std::string n = "") : name(std::move(n)) {}

    template<class Archive>
    void serialize(Archive& archive) {
        archive(CEREAL_NVP(name), CEREAL_NVP(files), CEREAL_NVP(subdirectories));
    }
};

class FileSystemDevice {
public:
    std::unique_ptr<Directory> root_directory;
    const std::string persistence_file = "simulated_hdd.json";

    FileSystemDevice() {
        root_directory = std::make_unique<Directory>("root");
        load();
    }

    ~FileSystemDevice() {
        save();
    }

    void createFile(const std::string& parent_path, const std::string& file_name, const std::vector<std::string>& file_content);

    void listContents(const std::string& path);

private:
    void save();

    void load();

    // Simplified directory finding (recursive search needed for full path resolution)
    Directory* findDirectory(const std::string& path);
};

#endif // FILE_SYSTEM_DEVICE_H
