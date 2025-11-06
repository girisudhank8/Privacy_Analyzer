// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <ctime>
#include <iomanip>

DatabaseManager::DatabaseManager() {
    host = "127.0.0.1";          // localhost
    user = "cppuser";             // New user
    password = "cpppass";         // password you just set
    schema = "privacy_db";        // database name
    port = 3306;
    driver = nullptr;
    cout << "[DatabaseManager] Default constructor called." << endl;
}

DatabaseManager::DatabaseManager(string h, string u, string p, string s, unsigned int prt) {
    host = h;
    user = u;
    password = p;
    schema = s;
    port = prt;
    driver = nullptr;
    cout << "[DatabaseManager] Parameterized constructor called." << endl;
}

DatabaseManager::~DatabaseManager() {
    close();
    cout << "[DatabaseManager] Destructor called, connection closed." << endl;
}

bool DatabaseManager::connect() {
    try {
        driver = get_driver_instance();
        string uri = "tcp://" + host + ":" + to_string(port);
        conn.reset(driver->connect(uri, user, password));
        if (!conn) return false;
        conn->setSchema(schema);
        cout << "[DatabaseManager] Connected to database successfully!" << endl;
        return true;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Connection Error: " << e.what() << endl;
        return false;
    }
}

void DatabaseManager::close() {
    if (conn) {
        try {
            conn->close();
        } catch (...) {}
        conn.reset();
        cout << "[DatabaseManager] Connection closed." << endl;
    }
}

bool DatabaseManager::createPolicyTable() {
    if (!conn) {
        if (!connect()) return false;
    }

    try {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        string createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS stored_policies (
                id INT AUTO_INCREMENT PRIMARY KEY,
                content TEXT NOT NULL,
                source VARCHAR(50) NOT NULL,
                filename VARCHAR(255),
                char_count INT,
                analysis_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";
        
        stmt->execute(createTableSQL);
        cout << "[DatabaseManager] Policy table created/verified successfully." << endl;
        return true;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error creating table: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::createAnalysisTable() {
    if (!conn) {
        if (!connect()) return false;
    }

    try {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        string createTableSQL = R"(
            CREATE TABLE IF NOT EXISTS policy_analysis (
                id INT AUTO_INCREMENT PRIMARY KEY,
                policy_id INT NOT NULL,
                keyword_analysis TEXT NOT NULL,
                ai_summary TEXT,
                analysis_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (policy_id) REFERENCES stored_policies(id) ON DELETE CASCADE
            )
        )";
        
        stmt->execute(createTableSQL);
        cout << "[DatabaseManager] Analysis table created/verified successfully." << endl;
        return true;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error creating analysis table: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::storePolicy(const string& content, const string& source, const string& filename) {
    if (!conn) {
        if (!connect()) return false;
    }

    // Ensure table exists
    if (!createPolicyTable()) {
        return false;
    }

    try {
        string insertSQL = "INSERT INTO stored_policies (content, source, filename, char_count) VALUES (?, ?, ?, ?)";
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertSQL));
        
        pstmt->setString(1, content);
        pstmt->setString(2, source);
        pstmt->setString(3, filename);
        pstmt->setInt(4, content.length());
        
        pstmt->executeUpdate();
        cout << "[DatabaseManager] Policy stored successfully. Characters: " << content.length() << endl;
        return true;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error storing policy: " << e.what() << endl;
        return false;
    }
}

bool DatabaseManager::storeAnalysisResults(int policy_id, const string& keyword_analysis, const string& ai_summary) {
    if (!conn) {
        if (!connect()) return false;
    }

    // Ensure table exists
    if (!createAnalysisTable()) {
        return false;
    }

    try {
        string insertSQL = "INSERT INTO policy_analysis (policy_id, keyword_analysis, ai_summary) VALUES (?, ?, ?)";
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(insertSQL));
        
        pstmt->setInt(1, policy_id);
        pstmt->setString(2, keyword_analysis);
        pstmt->setString(3, ai_summary);
        
        pstmt->executeUpdate();
        cout << "[DatabaseManager] Analysis results stored successfully for policy ID: " << policy_id << endl;
        return true;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error storing analysis: " << e.what() << endl;
        return false;
    }
}

vector<PolicyRecord> DatabaseManager::getStoredPolicies() {
    vector<PolicyRecord> policies;
    
    if (!conn) {
        if (!connect()) return policies;
    }

    try {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(stmt->executeQuery(
            "SELECT id, content, source, filename, char_count, analysis_date FROM stored_policies ORDER BY analysis_date DESC"
        ));
        
        while (res->next()) {
            PolicyRecord record;
            record.id = res->getInt("id");
            record.content = res->getString("content");
            record.source = res->getString("source");
            record.filename = res->getString("filename");
            record.char_count = res->getInt("char_count");
            record.analysis_date = res->getString("analysis_date");
            
            policies.push_back(record);
        }
        cout << "[DatabaseManager] Retrieved " << policies.size() << " stored policies." << endl;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error retrieving policies: " << e.what() << endl;
    }

    return policies;
}

vector<AnalysisResult> DatabaseManager::getAnalysisResults(int policy_id) {
    vector<AnalysisResult> results;
    
    if (!conn) {
        if (!connect()) return results;
    }

    try {
        string querySQL = "SELECT policy_id, keyword_analysis, ai_summary, analysis_date FROM policy_analysis WHERE policy_id = ? ORDER BY analysis_date DESC";
        unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(querySQL));
        pstmt->setInt(1, policy_id);
        
        unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        
        while (res->next()) {
            AnalysisResult result;
            result.policy_id = res->getInt("policy_id");
            result.keyword_analysis = res->getString("keyword_analysis");
            result.ai_summary = res->getString("ai_summary");
            result.analysis_date = res->getString("analysis_date");
            
            results.push_back(result);
        }
        cout << "[DatabaseManager] Retrieved " << results.size() << " analysis results for policy ID: " << policy_id << endl;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error retrieving analysis: " << e.what() << endl;
    }

    return results;
}

vector<pair<string, string>> DatabaseManager::getKeywords() {
    vector<pair<string, string>> keywords;
    if (!conn) {
        cerr << "[DatabaseManager] Not connected to DB." << endl;
        return keywords;
    }

    try {
        unique_ptr<sql::Statement> stmt(conn->createStatement());
        unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT keyword, category FROM privacy_keywords"));
        while (res->next()) {
            string k = res->getString("keyword");
            string c = res->getString("category");
            keywords.push_back(make_pair(k, c));
        }
        cout << "[DatabaseManager] Keywords fetched: " << keywords.size() << endl;
    } catch (sql::SQLException &e) {
        lastError = e.what();
        cerr << "SQL Error: " << e.what() << endl;
    }

    return keywords;
}

string DatabaseManager::getLastError() const {
    return lastError;
}