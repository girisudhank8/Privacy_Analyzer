Privacy Policy Analyzer

**Privacy Policy Analyzer** is a C++ project designed to analyze and summarize privacy policies automatically.  
It detects key privacy-related terms, categorizes them (e.g., *Data Collection*, *Data Sharing*, *User Rights*),  
and generates concise AI-powered summaries using an integrated **LLM (Gemma 2B via Ollama)**.  
All analysis results are stored in a **MySQL database** for history tracking and reuse.

## 🚀 Features

- 🔍 **Keyword Detection** – Scans privacy policies for key privacy-related terms.  
- 🧠 **AI Summarization** – Generates intelligent summaries via Ollama’s Gemma 2B model.  
- 💾 **Database Integration** – Stores policies and analysis results in MySQL.  
- 📂 **File & Manual Input** – Supports both file input and manual entry.  
- 🧾 **History Tracking** – View stored policies and past analyses.  
- 🎨 **Color-Coded CLI** – User-friendly terminal interface with progress effects.

## 🧩 Project Structure
PrivacyPolicyAnalyzer/
├── main.cpp
├── DatabaseManager.h/.cpp
├── KeywordMatcher.h/.cpp
├── LLMManager.h/.cpp
├── TextAnalyzer.h/.cpp
└── README.md

## ⚙️ Dependencies

### 🧰 System Requirements
- C++17 or later  
- MySQL Server  
- `mysql-connector-c++`  
- `libcurl`  
- `Ollama` (for local LLM inference)

### 📦 Link Libraries
```bash
-lmysqlcppconn -lcurl -lpthread

Ollama Setup
ollama pull gemma:2b
ollama serve
Default API endpoint: http://localhost:11434

Database Setup
Start MySQL and create the database:

CREATE DATABASE privacy_db;

Create user and grant access:

CREATE USER 'cppuser'@'localhost' IDENTIFIED BY 'cpppass';
GRANT ALL PRIVILEGES ON privacy_db.* TO 'cppuser'@'localhost';
FLUSH PRIVILEGES;


Add the keywords table:

CREATE TABLE privacy_keywords (
    id INT AUTO_INCREMENT PRIMARY KEY,
    keyword VARCHAR(255),
    category VARCHAR(255)
);


Insert sample keywords:

INSERT INTO privacy_keywords (keyword, category) VALUES
('collect', 'Data Collection'),
('share', 'Data Sharing'),
('consent', 'User Rights'),
('delete', 'User Rights'),
('third party', 'Data Sharing');


The program auto-creates:

stored_policies

policy_analysis

🖥️ Usage
🧮 Compile
g++ main.cpp DatabaseManager.cpp KeywordMatcher.cpp LLMManager.cpp TextAnalyzer.cpp -o analyzer -lmysqlcppconn -lcurl -lpthread

▶️ Run
./analyzer
