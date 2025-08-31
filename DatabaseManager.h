// DatabaseManager.h
#include <pqxx/pqxx>
#include <string>

class DatabaseManager {
private:
    pqxx::connection m_conn;

public:
    DatabaseManager(const std::string& conn_info);
    ~DatabaseManager();

  void logEvent(int session_id, const std::string& event_type, const std::string& payload);

  int createSession(const std::string& programName);

  void saveSnapshot(int session_id, const std::string& snapshotData);

  void log(int session_id, const std::string& message, const std::string& level,
	   uint64_t instruction_pointer, const std::string& source_file,
	   int source_line) ;

};

