
#include "TextAnalyzer.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;


#define RESET   "\033[0m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BLUE    "\033[34m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"

void showTitle() {
    cout << CYAN << "=============================================\n";
    cout << "           Privacy Policy Analyzer           \n";
    cout << "=============================================" << RESET << "\n\n";
}

void loadingEffect(string message) {
    cout << message;
    for (int i = 0; i < 3; ++i) {
        cout << ".";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(400));
    }
    cout << endl;
}

void showStoredPolicies(TextAnalyzer& analyzer) {
    vector<PolicyRecord> policies = analyzer.getStoredPolicies();
    
    if (policies.empty()) {
        cout << YELLOW << "No stored policies found.\n" << RESET;
        return;
    }
    
    cout << GREEN << "\n Stored Privacy Policies:\n";
    cout << "==========================\n" << RESET;
    
    for (const auto& policy : policies) {
        cout << "ID: " << policy.id << "\n";
        cout << "Source: " << policy.source;
        if (!policy.filename.empty()) {
            cout << " (" << policy.filename << ")";
        }
        cout << "\n";
        cout << "Characters: " << policy.char_count << "\n";
        cout << "Date: " << policy.analysis_date << "\n";
        
        // Show preview of content (first 100 chars)
        string preview = policy.content.substr(0, 100);
        if (policy.content.length() > 100) {
            preview += "...";
        }
        cout << "Preview: " << preview << "\n";
        
        // shows analysis history
        vector<AnalysisResult> analyses = analyzer.getAnalysisHistory(policy.id);
        cout << "Analyses: " << analyses.size() << " time(s)\n";
        
        cout << "--------------------------\n";
    }
}

void showAnalysisHistory(TextAnalyzer& analyzer) {
    int policy_id;
    cout << YELLOW << "Enter policy ID to view analysis history: " << RESET;
    cin >> policy_id;
    cin.ignore(); // flush newline
    
    vector<AnalysisResult> analyses = analyzer.getAnalysisHistory(policy_id);
    
    if (analyses.empty()) {
        cout << RED << "No analysis history found for policy ID: " << policy_id << "\n" << RESET;
        return;
    }
    
    cout << GREEN << "\n Analysis History for Policy ID: " << policy_id << "\n";
    cout << "=====================================\n" << RESET;
    
    for (size_t i = 0; i < analyses.size(); i++) {
        const auto& analysis = analyses[i];
        cout << "Analysis #" << (i + 1) << " - " << analysis.analysis_date << "\n";
        cout << "-------------------------------------\n";
        
        if (!analysis.keyword_analysis.empty()) {
            cout << "Keyword Analysis:\n";
            cout << analysis.keyword_analysis << "\n";
        }
        
        if (!analysis.ai_summary.empty()) {
            cout << "AI Summary:\n";
            cout << analysis.ai_summary << "\n";
        }
        
        cout << "\n";
    }
}

int main() {
    showTitle();

    TextAnalyzer analyzer;
    int choice;
    string filename;
    string text;

    do {
        cout << MAGENTA << "\n---------- MENU ----------" << RESET << endl;
        cout << "1. Load text from file" << endl;
        cout << "2. Enter text manually" << endl;
        cout << "3. Analyze privacy policy" << endl;
        cout << "4. Generate AI summary" << endl;
        cout << "5. Store current policy in database" << endl;
        cout << "6. Store analysis results in database" << endl;
        cout << "7. View stored policies" << endl;
        cout << "8. View analysis history" << endl;
        cout << "9. Exit" << endl;
        cout << "--------------------------" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(); // flush newline

        switch (choice) {
            case 1:
                cout << YELLOW << "Enter file name (e.g., policy.txt): " << RESET;
                getline(cin, filename);
                loadingEffect("Loading file");
                if (analyzer.loadFromFile(filename)) {
                    cout << GREEN << " File loaded successfully!\n" << RESET;
                } else {
                    cout << RED << " Failed to load file.\n" << RESET;
                }
                break;

            case 2:
                cout << GREEN << "Enter your privacy policy text below:\n" << RESET;
                getline(cin, text);
                analyzer.loadText(text);
                cout << GREEN << " Text loaded successfully!\n" << RESET;
                break;

            case 3:
                loadingEffect("Analyzing");
                analyzer.analyze();
                break;

            case 4:
                cout << YELLOW << "Generating AI-powered summary..." << RESET << endl;
                loadingEffect("Consulting AI");
                {
                    string summary = analyzer.generateSummary();
                    cout << summary << endl;
                    
                    // asks if user wants to store the analysis
                    /* if (summary.find("Error:") == string::npos) {
                        cout << "Store this analysis in database? (y/n): ";
                        char store_choice;
                        cin >> store_choice;
                        cin.ignore();
                        if (store_choice == 'y' || store_choice == 'Y') {
                            if (analyzer.storeAnalysisResults(summary)) {
                                cout << GREEN << " Analysis stored successfully!\n" << RESET;
                            } else {
                                cout << RED << " Failed to store analysis.\n" << RESET;
                            }
                        }
                    }*/
                }
                break;

            case 5:
                loadingEffect("Storing policy in database");
                if (analyzer.storeCurrentPolicy()) {
                    cout << GREEN << " Policy stored successfully in database!\n" << RESET;
                } else {
                    cout << RED << " Failed to store policy.\n" << RESET;
                }
                break;

            case 6:
                loadingEffect("Storing analysis results");
                if (analyzer.storeAnalysisResults()) {
                    cout << GREEN << " Analysis results stored successfully!\n" << RESET;
                } else {
                    cout << RED << " Failed to store analysis results.\n" << RESET;
                }
                break;

            case 7:
                showStoredPolicies(analyzer);
                break;

            case 8:
                showAnalysisHistory(analyzer);
                break;

            case 9:
                cout << BLUE << "Exiting program. Goodbye!" << RESET << endl;
                break;

            default:
                cout << RED << "Invalid choice. Try again.\n" << RESET;
        }

    } while (choice != 9);

    return 0;
}