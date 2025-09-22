#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "i_database_manager.h"
#include <pqxx/pqxx>
#include <string>

class DatabaseManager : public IDatabaseManager {
private:
  pqxx::connection m_conn;

public:
  DatabaseManager(const std::string &conn_info);
  ~DatabaseManager() override;

  void logEvent(int session_id, const std::string &event_type,
                const std::string &payload) override;

  int createSession(const std::string &programName) override;

  void saveSnapshot(int session_id, const std::string &snapshotData) override;

  void log(int session_id, const std::string &message, const std::string &level,
           uint64_t instruction_pointer, const std::string &source_file,
           int source_line) override;
};

#endif // DATABASE_MANAGER_H
