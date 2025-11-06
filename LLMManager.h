// LLMManager.h
#ifndef LLMMANAGER_H
#define LLMMANAGER_H

#include <iostream>
#include <string>

class LLMManager {
private:
    std::string apiUrl;
    std::string modelName;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);

public:
    LLMManager(const std::string& url = "http://localhost:11434", const std::string& model = "gemma:2b");
    
    // Generate summary with optional keyword analysis
    std::string generateSummary(const std::string& policyText, const std::string& keywordAnalysis = "");
    
    bool isServerAvailable();
    
    virtual ~LLMManager();
};

#endif