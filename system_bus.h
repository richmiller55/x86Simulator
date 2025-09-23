#ifndef SYSTEM_BUS_H
#define SYSTEM_BUS_H

#include "i_database_manager.h"
#include "file_system_device.h"
#include "x86_simulator.h"
#include <vector>
#include <string>
#include <memory>
#include <map>

struct DeviceInfo {
    std::string name;
    std::string type;
    std::map<std::string, std::string> properties;
};

class SystemBus {
public:
    SystemBus(IDatabaseManager& db_manager);
    ~SystemBus();

    void load_configuration(const std::string& config_path);
    void run();

    size_t get_process_count() const;
    const X86Simulator* get_process(size_t index) const;

private:
    IDatabaseManager& db_manager_;
    std::vector<std::unique_ptr<X86Simulator>> processes_;
    std::vector<DeviceInfo> devices_;
    std::unique_ptr<FileSystemDevice> file_system_;
};

#endif // SYSTEM_BUS_H
