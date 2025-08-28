// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& conn_info)
    : m_conn(conn_info)
{
    if (m_conn.is_open()) {
        std::cout << "Connected to database successfully." << std::endl;
    } else {
        throw std::runtime_error("Failed to connect to the database.");
    }
}

DatabaseManager::~DatabaseManager() {
    if (m_conn.is_open()) {
        m_conn.disconnect();
    }
}

void DatabaseManager::logEvent(const std::string& event_type, const std::string& payload) {
    try {
        pqxx::work txn(m_conn);
        txn.exec_params("INSERT INTO events (event_type, payload) VALUES ($1, $2)", event_type, payload);
        txn.commit();
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query: " << e.query() << std::endl;
    }
}
