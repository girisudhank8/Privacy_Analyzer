// KeywordMatcher.cpp
#include "KeywordMatcher.h"
#include <regex>
#include <sstream>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"

KeywordMatcher::KeywordMatcher() : DatabaseManager() {
    cout << "[KeywordMatcher] Ready to match privacy policy text.\n";
}

bool KeywordMatcher::loadKeywords() {
    if (!connect()) {
        cerr << "[KeywordMatcher] Could not connect to DB. Error: " << getLastError() << endl;
        return false;
    }

    keywordList = DatabaseManager::getKeywords();
    if (keywordList.empty()) {
        cerr << "[KeywordMatcher] No keywords found in DB.\n";
        return false;
    }

    cout << "[KeywordMatcher] Loaded " << keywordList.size() << " keywords from DB.\n";
    return true;
}

void KeywordMatcher::findMatches(const string &text) {
    if (keywordList.empty()) {
        cerr << "[KeywordMatcher] No keywords loaded.\n";
        return;
    }

    // Clear previous matches
    categoryCount.clear();
    matchedKeywordsByCategory.clear();

    cout << "\n  Analyzing Privacy Policy Text...\n";
    cout << "------------------------------------\n";

    for (auto &pair : keywordList) {
        string keyword = pair.first;
        string category = pair.second;

        // Case-insensitive regex search
        regex wordRegex("\\b" + keyword + "\\b", regex_constants::icase);
        smatch match;

        string::const_iterator searchStart(text.cbegin());
        bool found = false;

        while (regex_search(searchStart, text.cend(), match, wordRegex)) {
            found = true;
            searchStart = match.suffix().first;
            categoryCount[category]++;
            
            // Store the matched keyword
            matchedKeywordsByCategory[category].push_back(keyword);

            if (category == "Data Collection")
                cout << RED << "[" << keyword << "]" << RESET << " ";
            else if (category == "Data Sharing")
                cout << YELLOW << "[" << keyword << "]" << RESET << " ";
            else
                cout << GREEN << "[" << keyword << "]" << RESET << " ";
        }

        if (found)
            cout << " -> (" << category << ")\n";
    }
}

void KeywordMatcher::showSummary() {
    cout << "\n  Summary of Detected Terms:\n";
    cout << "------------------------------------\n";

    for (auto &entry : categoryCount) {
        cout << "Category: " << entry.first
             << " | Occurrences: " << entry.second << endl;
    }
}

map<string, vector<string>> KeywordMatcher::getMatchedKeywords() const {
    return matchedKeywordsByCategory;
}

string KeywordMatcher::getKeywordAnalysis() const {
    stringstream analysis;
    
    analysis << "KEYWORD ANALYSIS RESULTS:\n";
    analysis << "=========================\n";
    
    if (matchedKeywordsByCategory.empty()) {
        analysis << "No keywords matched in the privacy policy.\n";
        return analysis.str();
    }
    
    for (const auto& category : matchedKeywordsByCategory) {
        analysis << "\n" << category.first << ":\n";
        analysis << "  Occurrences: " << categoryCount.at(category.first) << "\n";
        analysis << "  Keywords found: ";
        
        // Remove duplicates and count frequencies
        map<string, int> keywordFreq;
        for (const auto& keyword : category.second) {
            keywordFreq[keyword]++;
        }
        
        bool first = true;
        for (const auto& kw : keywordFreq) {
            if (!first) analysis << ", ";
            analysis << kw.first;
            if (kw.second > 1) {
                analysis << "(" << kw.second << "x)";
            }
            first = false;
        }
        analysis << "\n";
    }
    
    return analysis.str();
}

vector<pair<string, string>> KeywordMatcher::getKeywords() {
    cout << "[KeywordMatcher] Overridden getKeywords() called.\n";
    return DatabaseManager::getKeywords();
}