// TextAnalyzer.h
#ifndef TEXTANALYZER_H
#define TEXTANALYZER_H

#include "KeywordMatcher.h"
#include "LLMManager.h"
#include <sstream>
#include <iostream>

using namespace std;

class TextAnalyzer {
protected:
    string policyText;
    KeywordMatcher matcher;
    LLMManager llmManager;
    string lastKeywordAnalysis;
    string currentSource; // Track where the current text came from
    string currentFilename; // Track filename if loaded from file

public:
    TextAnalyzer();

    // Load text directly
    virtual void loadText(const string &text);

    // Or load from file
    virtual bool loadFromFile(const string &filename);

    // Analyze the text
    virtual void analyze();

    // TextAnalyzer.h - Add to the public section
    // Store analysis results for current policy
    virtual bool storeAnalysisResults(const string& ai_summary = "");
    
    // Get analysis history for a specific policy
    virtual vector<AnalysisResult> getAnalysisHistory(int policy_id);
    
    // Get the last stored policy ID (for linking analysis)
    virtual int getLastStoredPolicyId();

    // Generate a short summary based on keyword stats
    virtual string generateSummary();

    // Store current policy in database
    virtual bool storeCurrentPolicy();

    // Get stored policies from database
    virtual vector<PolicyRecord> getStoredPolicies();

    // Get the keyword matcher for access to analysis
    KeywordMatcher& getMatcher() { return matcher; }

    virtual ~TextAnalyzer();
};

#endif