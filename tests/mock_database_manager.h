#ifndef MOCK_DATABASE_MANAGER_H
#define MOCK_DATABASE_MANAGER_H

#include "../DatabaseManager.h"

class MockDatabaseManager {
public:
  MockDatabaseManager();
  ~MockDatabaseManager() = default;

    void logEvent(int session_id, const std::string& event_type, const std::string& payload) {}
    int createSession(const std::string& programName) { return 1; }
    void saveSnapshot(int session_id, const std::string& snapshotData) {}
    void log(int session_id, const std::string& message, const std::string& level,
	   uint64_t instruction_pointer, const std::string& source_file,
	   int source_line) {}
};

#endif // MOCK_DATABASE_MANAGER_H
