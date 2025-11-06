// LLMManager.cpp
#include "LLMManager.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>

// Callback function for curl response
size_t LLMManager::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

LLMManager::LLMManager(const std::string& url, const std::string& model) 
    : apiUrl(url), modelName(model) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "[LLMManager] Initialized with model: " << modelName << std::endl;
}

LLMManager::~LLMManager() {
    curl_global_cleanup();
}

bool LLMManager::isServerAvailable() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    std::string response;
    std::string testUrl = apiUrl + "/api/tags";
    
    curl_easy_setopt(curl, CURLOPT_URL, testUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK);
}

std::string LLMManager::generateSummary(const std::string& policyText, const std::string& keywordAnalysis) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if(!curl) {
        return "Error: Failed to initialize CURL";
    }

    std::string url = apiUrl + "/api/generate";
    
    // Drastically reduce text length for Gemma 2B
    size_t maxLength = 500; // Much smaller for Gemma 2B
    std::string textToAnalyze = policyText.substr(0, maxLength);
    
    // Simple cleaning
    std::string cleanText;
    for (char c : textToAnalyze) {
        if (c == '\r' || c == '\n') {
            cleanText += ' ';
        } else if (c == '"') {
            cleanText += '\'';
        } else {
            cleanText += c;
        }
    }
    
    // Remove extra spaces
    std::string finalText;
    bool lastWasSpace = false;
    for (char c : cleanText) {
        if (c == ' ') {
            if (!lastWasSpace) {
                finalText += ' ';
                lastWasSpace = true;
            }
        } else {
            finalText += c;
            lastWasSpace = false;
        }
    }
    
    // Extract key information from keyword analysis for the prompt
    std::string keywordHint = "";
    if (!keywordAnalysis.empty()) {
        // Just mention that keywords were found, don't include the full analysis
        if (keywordAnalysis.find("Data Collection") != std::string::npos) {
            keywordHint = " Focus on data collection practices.";
        }
        if (keywordAnalysis.find("Data Sharing") != std::string::npos) {
            keywordHint += " Focus on data sharing practices.";
        }
        if (keywordAnalysis.find("User Rights") != std::string::npos) {
            keywordHint += " Focus on user rights.";
        }
    }
    
    // Very simple prompt for Gemma 2B
    std::string prompt = "Summarize this privacy policy in 3-4 sentences:" + keywordHint + " Text: " + finalText;

    std::cout << "[LLMManager] Using simplified prompt for Gemma 2B" << std::endl;
    std::cout << "[LLMManager] Prompt length: " << prompt.length() << std::endl;

    // Simple JSON payload
    std::string jsonPayload = "{\"model\":\"gemma:2b\",\"prompt\":\"" + prompt + "\",\"stream\":false}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L); // Reduced timeout
    
    // Disable verbose output for cleaner logs
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    std::cout << "[LLMManager] Sending request to LLM (timeout: 120s)..." << std::endl;
    res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if(res != CURLE_OK) {
        std::string error = "Error: CURL failed - ";
        error += curl_easy_strerror(res);
        std::cout << "[LLMManager] " << error << std::endl;
        return error;
    }

    if (http_code != 200) {
        std::string error = "Error: HTTP " + std::to_string(http_code);
        std::cout << "[LLMManager] " << error << std::endl;
        std::cout << "[LLMManager] Response: " << response << std::endl;
        return error;
    }

    // Parse response
    if (response.empty()) {
        return "Error: Empty response from LLM server";
    }

    // Simple JSON parsing
    size_t responsePos = response.find("\"response\":\"");
    if (responsePos != std::string::npos) {
        responsePos += 12;
        size_t endPos = response.find("\"", responsePos);
        if (endPos != std::string::npos) {
            std::string extracted = response.substr(responsePos, endPos - responsePos);
            
            // Basic unescaping
            std::string result;
            for (size_t i = 0; i < extracted.length(); i++) {
                if (extracted[i] == '\\' && i + 1 < extracted.length()) {
                    if (extracted[i+1] == 'n') {
                        result += '\n';
                        i++;
                    } else if (extracted[i+1] == '\\' || extracted[i+1] == '"') {
                        result += extracted[i+1];
                        i++;
                    } else {
                        result += extracted[i];
                    }
                } else {
                    result += extracted[i];
                }
            }
            
            std::cout << "[LLMManager] Successfully generated summary" << std::endl;
            return result;
        }
    }

    return "Error: Could not parse LLM response";
}