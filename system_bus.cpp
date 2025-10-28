#include "system_bus.h"
#include "x86_simulator.h"
#include "arm_simulator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

SystemBus::SystemBus(IDatabaseManager& db_manager) 
    : db_manager_(db_manager), file_system_(std::make_unique<FileSystemDevice>()) {
    std::cout << "db_manager_ address in SystemBus constructor: " << &db_manager_ << std::endl;
}

SystemBus::~SystemBus() {
}

bool SystemBus::load_configuration(const std::string& config_path) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Error: Could not open configuration file " << config_path << std::endl;
        return false;
    }

    json config;
    try {
        config_file >> config;
    } catch (json::parse_error& e) {
        std::cerr << "Error: Could not parse configuration file " << config_path << ": " << e.what() << std::endl;
        return false;
    }

    bool ui_enabled = config.value("ui_enabled", false);

    if (config.contains("processes")) {
        for (const auto& process_info : config["processes"]) {
            create_and_configure_simulator(process_info, ui_enabled);
        }
    }

    if (config.contains("devices")) {
        for (const auto& device_info_json : config["devices"]) {
            load_device_info(device_info_json);
        }
    }
    return true;
}

void SystemBus::run() {
    if (!processes_.empty()) {
        // For now, run the first process
        processes_[0].simulator->runProgram();
    }
}

size_t SystemBus::get_process_count() const {
    return processes_.size();
}

const ISimulator* SystemBus::get_process(size_t index) const {
    if (index < processes_.size()) {
        return processes_[index].simulator.get();
    }
    return nullptr;
}

// Private helper implementations

void SystemBus::create_and_configure_simulator(const json& process_info, bool ui_enabled) {
    std::string program_path = process_info.value("path", "");
    std::string isa_str = process_info.value("isa", "x86");

    int session_id = db_manager_.createSession(program_path);
    auto memory = std::make_unique<Memory>();
    memory->reset(); // Reset memory before use
    std::unique_ptr<ISimulator> simulator;

    if (isa_str == "x86") {
        auto x86_sim = std::make_unique<X86Simulator>(db_manager_, *memory, session_id, !ui_enabled);
        if (!x86_sim->loadProgram(program_path)) {
            std::cerr << "Error: Failed to load program '" << program_path << "'" << std::endl;
            return;
        }
        if (!x86_sim->firstPass()) {
            std::cerr << "Error: First pass failed for program '" << program_path << "'" << std::endl;
            return;
        }
        if (!x86_sim->secondPass()) {
            std::cerr << "Error: Second pass failed for program '" << program_path << "'" << std::endl;
            return;
        }
        // x86_sim->dumpTextSegment("text_segment.dump");
        // x86_sim->dumpDataSegment("data_segment.dump");
        // x86_sim->dumpSymbolTable("symbol_table.dump");
        simulator = std::move(x86_sim);
    } else if (isa_str == "arm") {
        auto arm_sim = std::make_unique<ArmSimulator>(db_manager_, *memory, session_id, !ui_enabled);
        arm_sim->loadProgram(program_path);
        simulator = std::move(arm_sim);
    } else {
        std::cerr << "Error: Unknown ISA '" << isa_str << "' in configuration." << std::endl;
        return;
    }

    Process process;
    process.memory = std::move(memory);
    process.simulator = std::move(simulator);
    processes_.push_back(std::move(process));
}

void SystemBus::load_device_info(const json& device_info_json) {
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
