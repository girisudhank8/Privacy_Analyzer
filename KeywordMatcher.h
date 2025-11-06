// KeywordMatcher.h
#ifndef KEYWORDMATCHER_H
#define KEYWORDMATCHER_H

#include "DatabaseManager.h"
#include <map>
#include <regex>
#include <vector>

using namespace std;

class KeywordMatcher : public DatabaseManager {
private:
    vector<pair<string, string>> keywordList; // from DB
    map<string, int> categoryCount;           // count matches by category
    map<string, vector<string>> matchedKeywordsByCategory; // store actual matched keywords

public:
    KeywordMatcher();

    // Loads keywords using parent class method
    bool loadKeywords();

    // Finds matches in text
    virtual void findMatches(const string &text);

    // Displays category summary
    virtual void showSummary();

    // Get matched keywords for LLM summary
    map<string, vector<string>> getMatchedKeywords() const;

    // Get formatted analysis for LLM
    string getKeywordAnalysis() const;

    // Override getKeywords() for demonstration
    vector<pair<string, string>> getKeywords() override;
};

#endif