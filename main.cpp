#include "system_bus.h"
#include "DatabaseManager.h"
#include "i_database_manager.h"
#include <iostream>
#include <cstdlib>

int main() {
    const char* conn_str_env = std::getenv("DB_CONN_STR");
    if (conn_str_env == nullptr) {
        std::cerr << "Error: DB_CONN_STR environment variable not set." << std::endl;
        return 1;
    }
    DatabaseManager dbManager(conn_str_env);
    SystemBus system_bus(dbManager);
    system_bus.load_configuration("system_bus.json");
    system_bus.run();
    return 0;
}
