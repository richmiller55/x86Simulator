#include "mock_database_manager.h"

MockDatabaseManager::MockDatabaseManager() {}

MockDatabaseManager::~MockDatabaseManager() {}

void MockDatabaseManager::logEvent(int session_id, const std::string& event_type, const std::string& payload) {
    // Mock implementation, does nothing.
}

int MockDatabaseManager::createSession(const std::string& programName) {
    return 1; // Return a dummy session ID.
}

void MockDatabaseManager::saveSnapshot(int session_id, const std::string& snapshotData) {
    // Mock implementation, does nothing.
}

void MockDatabaseManager::log(int session_id, const std::string& message, const std::string& level, uint64_t instruction_pointer, const std::string& source_file, int source_line) {
    // Mock implementation, does nothing.
}
