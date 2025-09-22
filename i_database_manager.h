#ifndef I_DATABASE_MANAGER_H
#define I_DATABASE_MANAGER_H

#include <string>
#include <cstdint>

class IDatabaseManager {
public:
    virtual ~IDatabaseManager() = default;
    virtual void logEvent(int session_id, const std::string& event_type, const std::string& payload) = 0;
    virtual int createSession(const std::string& programName) = 0;
    virtual void saveSnapshot(int session_id, const std::string& snapshotData) = 0;
    virtual void log(int session_id, const std::string& message, const std::string& level,
                     uint64_t instruction_pointer, const std::string& source_file,
                     int source_line) = 0;
};

#endif // I_DATABASE_MANAGER_H
