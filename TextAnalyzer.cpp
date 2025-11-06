// TextAnalyzer.cpp
#include "TextAnalyzer.h"
#include <fstream>
#include <iostream>

TextAnalyzer::TextAnalyzer() {
    cout << "[TextAnalyzer] Ready to analyze privacy policy text.\n";
    
    // Check if LLM server is available
    if (llmManager.isServerAvailable()) {
        cout << "[TextAnalyzer] LLM server connected successfully.\n";
    } else {
        cout << "[TextAnalyzer] Warning: LLM server not available. Using fallback summary.\n";
    }
    
    // Initialize source tracking
    currentSource = "";
    currentFilename = "";
}

void TextAnalyzer::loadText(const string &text) {
    policyText = text;
    currentSource = "manual";
    currentFilename = "";
    cout << "[TextAnalyzer] Text loaded (" << policyText.size() << " characters).\n";
}

bool TextAnalyzer::loadFromFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[TextAnalyzer] Could not open file: " << filename << endl;
        return false;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    policyText = buffer.str();
    currentSource = "file";
    currentFilename = filename;

    cout << "[TextAnalyzer] File loaded successfully: " << filename << endl;
    return true;
}

void TextAnalyzer::analyze() {
    if (policyText.empty()) {
        cerr << "[TextAnalyzer] No text loaded. Please load text first.\n";
        return;
    }

    if (!matcher.loadKeywords()) {
        cerr << "[TextAnalyzer] Could not load keywords from DB.\n";
        return;
    }

    cout << "\n Starting keyword analysis...\n";
    matcher.findMatches(policyText);
    matcher.showSummary();
    
    // Store the detailed keyword analysis for LLM
    lastKeywordAnalysis = matcher.getKeywordAnalysis();
    
    cout << " Analysis completed! Keyword data stored for AI summary.\n";
}

string TextAnalyzer::generateSummary() {
    if (policyText.empty()) {
        return "Error: No privacy policy text loaded. Please load text first.";
    }
    
    cout << " Generating AI-powered summary based on keyword analysis...\n";
    cout << "This may take 10-20 seconds...\n";
    
    // Check if LLM server is available
    if (!llmManager.isServerAvailable()) {
        stringstream summary;
        summary << "\n  LLM Server Not Available\n";
        summary << "==========================\n";
        summary << "Ollama is not responding. Please ensure:\n";
        summary << "1. Ollama is running: 'ollama serve'\n";
        summary << "2. Server is accessible at http://localhost:11434\n\n";
        summary << "Using keyword analysis instead:\n";
        summary << "------------------------------------\n";
        summary << lastKeywordAnalysis;
        return summary.str();
    }
    
    // Generate summary using LLM with keyword analysis
    string llmSummary = llmManager.generateSummary(policyText, lastKeywordAnalysis);
    
    if (llmSummary.find("Error:") == 0) {
        cout << "[LLM] Generation failed: " << llmSummary << endl;
        stringstream summary;
        summary << "\n  LLM Generation Issue\n";
        summary << "======================\n";
        summary << "Failed to generate AI summary.\n\n";
        summary << "Using keyword analysis instead:\n";
        summary << "------------------------------------\n";
        summary << lastKeywordAnalysis;
        return summary.str();
    }
    
    stringstream summary;
    summary << "\n AI-Powered Privacy Policy Summary (Based on Keyword Analysis)\n";
    summary << "==============================================================\n";
    summary << llmSummary;
    summary << "\n==============================================================\n";
    summary << "\nNote: This AI analysis is based on detected privacy-related keywords and should be verified by legal experts.\n";
    return summary.str();
}

bool TextAnalyzer::storeCurrentPolicy() {
    if (policyText.empty()) {
        cerr << "[TextAnalyzer] No policy text to store.\n";
        return false;
    }
    
    if (currentSource.empty()) {
        cerr << "[TextAnalyzer] No source information available.\n";
        return false;
    }
    
    // Use the DatabaseManager from matcher to store the policy
    bool success = matcher.storePolicy(policyText, currentSource, currentFilename);
    if (success) {
        cout << "[TextAnalyzer] Policy stored in database successfully.\n";
    } else {
        cerr << "[TextAnalyzer] Failed to store policy in database.\n";
    }
    return success;
}

vector<PolicyRecord> TextAnalyzer::getStoredPolicies() {
    return matcher.getStoredPolicies();
}

bool TextAnalyzer::storeAnalysisResults(const string& ai_summary) {
    if (lastKeywordAnalysis.empty()) {
        cerr << "[TextAnalyzer] No analysis results to store. Please analyze the policy first.\n";
        return false;
    }
    
    // Get the last stored policy ID
    vector<PolicyRecord> policies = getStoredPolicies();
    if (policies.empty()) {
        cerr << "[TextAnalyzer] No stored policies found. Please store the policy first.\n";
        return false;
    }
    
    int latest_policy_id = policies[0].id; // Get the latest policy ID
    
    bool success = matcher.storeAnalysisResults(latest_policy_id, lastKeywordAnalysis, ai_summary);
    if (success) {
        cout << "[TextAnalyzer] Analysis results stored successfully for policy ID: " << latest_policy_id << endl;
    } else {
        cerr << "[TextAnalyzer] Failed to store analysis results.\n";
    }
    return success;
}

vector<AnalysisResult> TextAnalyzer::getAnalysisHistory(int policy_id) {
    return matcher.getAnalysisResults(policy_id);
}

int TextAnalyzer::getLastStoredPolicyId() {
    vector<PolicyRecord> policies = getStoredPolicies();
    if (policies.empty()) {
        return -1;
    }
    return policies[0].id; // Return the latest policy ID
}

TextAnalyzer::~TextAnalyzer() {
    cout << "[TextAnalyzer] Analysis completed and resources cleared.\n";
}