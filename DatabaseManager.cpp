// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& conn_info)
    : m_conn(conn_info){
    if (m_conn.is_open()) {
        std::cout << "Connected to database successfully." << std::endl;
    } else {
        throw std::runtime_error("Failed to connect to the database.");
    }
}

DatabaseManager::~DatabaseManager() = default; // This is now correct because it's declared in the header.

void DatabaseManager::logEvent(int session_id, const std::string& event_type, const std::string& payload) {
    try {
        pqxx::work txn(m_conn);
	pqxx::params p;
	p.append(event_type);
	p.append(payload);
        txn.exec("INSERT INTO events (event_type, payload) VALUES ($1, $2)",p);
        txn.commit();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query: " << e.query() << std::endl;
    }
};

void DatabaseManager::log(int session_id,
			  const std::string& message, 
			  const std::string& level,
			  uint64_t instruction_pointer,
			  const std::string& source_file,
			  int source_line) {
        
        try {
            pqxx::work w(m_conn);
	    pqxx::params p;
	    p.append(session_id);
	    p.append(instruction_pointer);
	    p.append(level);
	    p.append(message);
	    p.append(source_file);
	    p.append(source_line);

            w.exec(
                "INSERT INTO log_entries (session_id, timestamp, instruction_pointer, level, message, source_file, source_line) "
                "VALUES ($1, NOW(), $2, $3, $4, $5, $6);", p );
            w.commit();
        } catch (const std::exception& e) {
            std::cerr << "Database logging failed: " << e.what() << std::endl;
        }
}

int DatabaseManager::createSession(const std::string& program_name) {
  pqxx::work w(m_conn);
  pqxx::params p;
  p.append(program_name);
 
  pqxx::result r = w.exec(
			  "INSERT INTO simulation_session (start_time, program_name) "
			  "VALUES (NOW(), $1) "
			  "RETURNING session_id;", p);
  w.commit();

  // Check the result to get the session_id
  int session_id = r[0][0].as<int>();
  return session_id;
}
