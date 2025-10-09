#ifndef SYSTEM_BUS_H
#define SYSTEM_BUS_H

#include "i_database_manager.h"
#include "file_system_device.h"
#include "memory.h"
#include "x86_simulator.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <nlohmann/json.hpp>

struct DeviceInfo {
    std::string name;
    std::string type;
    std::map<std::string, std::string> properties;
};

struct Process {
    std::unique_ptr<Memory> memory;
    std::unique_ptr<X86Simulator> simulator;
};

class SystemBus {
public:
    SystemBus(IDatabaseManager& db_manager);
    ~SystemBus();

    bool load_configuration(const std::string& config_path);
    void run();

    size_t get_process_count() const;
    const X86Simulator* get_process(size_t index) const;

private:
    void create_and_configure_simulator(const nlohmann::json& process_info, bool ui_enabled);
    void load_device_info(const nlohmann::json& device_info_json);

    IDatabaseManager& db_manager_;
    std::vector<Process> processes_;
    std::vector<DeviceInfo> devices_;
    std::unique_ptr<FileSystemDevice> file_system_;
};

#endif // SYSTEM_BUS_H