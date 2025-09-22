#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

// DatabaseManager.h
#include <pqxx/pqxx>
#include <string>

class DatabaseManager {
private:
    pqxx::connection m_conn;

protected:
    DatabaseManager() = default; // For use by mock objects

public:
    DatabaseManager(const std::string& conn_info);
    virtual ~DatabaseManager();
    

  virtual void logEvent(int session_id, const std::string& event_type, const std::string& payload);

  virtual int createSession(const std::string& programName);

  virtual void saveSnapshot(int session_id, const std::string& snapshotData);

  virtual void log(int session_id, const std::string& message, const std::string& level,
	   uint64_t instruction_pointer, const std::string& source_file,
	   int source_line) ;

};

#endif // DATABASE_MANAGER_H
