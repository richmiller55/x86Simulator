#include "system_bus.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

SystemBus::SystemBus(IDatabaseManager& db_manager) : db_manager_(db_manager) {
    file_system_ = std::make_unique<FileSystemDevice>();
}

SystemBus::~SystemBus() {
}

void SystemBus::load_configuration(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Error: Could not open configuration file " << config_path << std::endl;
        return;
    }

    json config;
    try {
        config_file >> config;
    } catch (json::parse_error& e) {
        std::cerr << "Error: Could not parse configuration file " << config_path << ": " << e.what() << std::endl;
        return;
    }

    bool ui_enabled = false;
    if (config.contains("ui_enabled")) {
        ui_enabled = config["ui_enabled"].get<bool>();
    }

    if (config.contains("processes")) {
        for (const auto& process_info : config["processes"]) {
            std::string program_path = process_info["path"];
            int session_id = db_manager_.createSession(program_path);
            auto simulator = std::make_unique<X86Simulator>(db_manager_, session_id, !ui_enabled);
            simulator->loadProgram(program_path);
            simulator->firstPass();
            simulator->secondPass();
	    simulator->dumpTextSegment("text_segment_dump.txt");
	    simulator->dumpDataSegment("data_segment_dump.txt");
            processes_.push_back(std::move(simulator));
        }
    }

    if (config.contains("devices")) {
        for (const auto& device_info_json : config["devices"]) {
            DeviceInfo device_info;
            device_info.name = device_info_json["name"];
            device_info.type = device_info_json["type"];
            for (auto& [key, value] : device_info_json.items()) {
                if (key != "name" && key != "type") {
                    device_info.properties[key] = value;
                }
            }
            devices_.push_back(device_info);
        }
    }
}

void SystemBus::run() {
    if (!processes_.empty()) {
        // For now, run the first process
        processes_[0]->runProgram();
    }
}

size_t SystemBus::get_process_count() const {
    return processes_.size();
}

const X86Simulator* SystemBus::get_process(size_t index) const {
    if (index < processes_.size()) {
        return processes_[index].get();
    }
    return nullptr;
}
