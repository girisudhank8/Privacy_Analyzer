// DatabaseManager.h
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <stdexcept>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

// Structure to store policy information
struct PolicyRecord {
    int id;
    string content;
    string source; // "file" or "manual"
    string filename; // if from file
    size_t char_count;
    string analysis_date;
};

// Structure to store analysis results
struct AnalysisResult {
    int policy_id;
    string keyword_analysis;
    string ai_summary;
    string analysis_date;
};

class DatabaseManager {
protected:
    string host, user, password, schema;
    unsigned int port;
    sql::Driver *driver;
    unique_ptr<sql::Connection> conn;
    string lastError;

public:
    // Default constructor
    DatabaseManager();

    // Parameterized constructor
    DatabaseManager(string h, string u, string p, string s, unsigned int prt = 3306);

    // Virtual destructor for inheritance safety
    virtual ~DatabaseManager();

    // Virtual methods (can be overridden)
    virtual bool connect();
    virtual void close();
    virtual vector<pair<string, string>> getKeywords();
    
    // New methods for policy storage
    virtual bool storePolicy(const string& content, const string& source, const string& filename = "");
    virtual vector<PolicyRecord> getStoredPolicies();
    virtual bool createPolicyTable(); // Create table if not exists

    // New methods for analysis storage
    virtual bool storeAnalysisResults(int policy_id, const string& keyword_analysis, const string& ai_summary = "");
    virtual vector<AnalysisResult> getAnalysisResults(int policy_id);
    virtual bool createAnalysisTable(); // Create analysis table if not exists

    // Getter for error messages
    string getLastError() const;
};

#endif