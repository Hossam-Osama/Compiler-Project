#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring>  // Changed from <string.h> to <cstring>
#include <cctype>   // Added for character handling

using namespace std;

// ---------- Helper Functions ----------

// Improved trim function
string trim(const string &str) {
    if (str.empty()) return "";
    
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\n\r");
    if (last == string::npos) return str.substr(first);
    
    return str.substr(first, last - first + 1);
}

// Check if symbol is epsilon (empty string)
bool isEpsilon(const string &symbol) {
    string s = trim(symbol);
    return s == "\\L" || s == "epsilon" || s == "EPSILON" || s.empty();
}

// Check if symbol is a terminal
bool isTerminal(const string &symbol, const map<string, vector<vector<string>>> &grammar) {
    string s = trim(symbol);
    
    // Symbols in quotes are terminals (e.g., 'id', 'int')
    if (s.length() > 1 && s[0] == '\'' && s[s.length() - 1] == '\'') {
        return true;
    }
    
    // Symbols that are not non-terminals and not epsilon are terminals
    return !isEpsilon(s) && grammar.find(s) == grammar.end();
}

// String comparison function (if you really need it)
bool compareStringToKeyByStrCmp(const map<string, set<string>>& myMap, const string& key) {
    string trimmedKey = trim(key);
    
    for (const auto& entry : myMap) {
        string trimmedEntryKey = trim(entry.first);
        
        // Direct comparison is more efficient
        if (trimmedEntryKey == trimmedKey) {
            return true;
        }
        
        // Or use strcmp if you prefer
        // if (strcmp(trimmedEntryKey.c_str(), trimmedKey.c_str()) == 0) {
        //     return true;
        // }
    }
    return false;
}

// ---------- FIRST Calculation ----------

void calculateFirst(map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first) {
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 1000;  // Prevent infinite loop
    
    // Step 1: Initialize FIRST for terminals
    for (const auto &rule : grammar) {
        for (const auto &production : rule.second) {
            for (const auto &symbol : production) {
                string sym = trim(symbol);
                
                if (!sym.empty() && !isEpsilon(sym) && grammar.find(sym) == grammar.end()) {
                    // This is a terminal symbol
                    first[sym].insert(sym);
                }
            }
        }
    }
    
    // Step 2: Calculate FIRST for non-terminals
    while (changed && iteration < MAX_ITERATIONS) {
        changed = false;
        iteration++;
        
        for (const auto &rule : grammar) {
            const string &nonTerminal = rule.first;
            
            for (const auto &production : rule.second) {
                if (production.empty()) {
                    // Empty production means epsilon
                    if (first[nonTerminal].insert("\\L").second) {
                        changed = true;
                    }
                    continue;
                }
                
                bool allCanDeriveEpsilon = true;
                
                for (size_t i = 0; i < production.size(); ++i) {
                    string symbol = trim(production[i]);
                    
                    if (symbol.empty()) {
                        allCanDeriveEpsilon = false;
                        break;
                    }
                    
                    // Case 1: Epsilon symbol
                    if (isEpsilon(symbol)) {
                        if (first[nonTerminal].insert("\\L").second) {
                            changed = true;
                        }
                        break;
                    }
                    
                    // Case 2: Terminal symbol
                    if (isTerminal(symbol, grammar)) {
                        if (first[nonTerminal].insert(symbol).second) {
                            changed = true;
                        }
                        allCanDeriveEpsilon = false;
                        break;
                    }
                    
                    // Case 3: Non-terminal symbol
                    size_t beforeSize = first[nonTerminal].size();
                    
                    // Add FIRST(symbol) to FIRST(nonTerminal) - excluding epsilon
                    for (const auto &f : first[symbol]) {
                        if (!isEpsilon(f)) {
                            first[nonTerminal].insert(f);
                        }
                    }
                    
                    if (first[nonTerminal].size() != beforeSize) {
                        changed = true;
                    }
                    
                    // If symbol cannot derive epsilon, stop
                    if (first[symbol].find("\\L") == first[symbol].end()) {
                        allCanDeriveEpsilon = false;
                        break;
                    }
                }
                
                // If all symbols can derive epsilon
                if (allCanDeriveEpsilon) {
                    if (first[nonTerminal].insert("\\L").second) {
                        changed = true;
                    }
                }
            }
        }
    }
    
    if (iteration >= MAX_ITERATIONS) {
        cerr << "Warning: FIRST calculation reached maximum iterations (" << MAX_ITERATIONS << ")\n";
    }
}

// ---------- FOLLOW Calculation ----------

void calculateFollow(map<string, vector<vector<string>>> &grammar, 
                     map<string, set<string>> &first, 
                     map<string, set<string>> &follow, 
                     const string &startSymbol) {
    
    string start = trim(startSymbol);
    
    // Step 1: Add $ to FOLLOW(start symbol)
    follow[start].insert("$");
    
    bool changed = true;
    int iteration = 0;
    const int MAX_ITERATIONS = 1000;
    
    while (changed && iteration < MAX_ITERATIONS) {
        changed = false;
        iteration++;
        
        for (const auto &rule : grammar) {
            const string &A = rule.first;  // Left-hand side
            
            for (const auto &production : rule.second) {
                for (size_t i = 0; i < production.size(); ++i) {
                    string B = trim(production[i]);  // Current symbol
                    
                    // Skip terminals and epsilon
                    if (isTerminal(B, grammar) || isEpsilon(B)) {
                        continue;
                    }
                    
                    // Case 1: B is the last symbol in the production
                    bool isLastSymbol = true;
                    for (size_t j = i + 1; j < production.size(); ++j) {
                        string next = trim(production[j]);
                        if (!next.empty() && !isEpsilon(next)) {
                            isLastSymbol = false;
                            break;
                        }
                    }
                    
                    if (isLastSymbol) {
                        size_t beforeSize = follow[B].size();
                        follow[B].insert(follow[A].begin(), follow[A].end());
                        if (follow[B].size() != beforeSize) {
                            changed = true;
                        }
                    }
                    
                    // Case 2: There are symbols after B
                    if (i + 1 < production.size()) {
                        for (size_t j = i + 1; j < production.size(); ++j) {
                            string nextSymbol = trim(production[j]);
                            
                            if (nextSymbol.empty() || isEpsilon(nextSymbol)) {
                                continue;
                            }
                            
                            if (isTerminal(nextSymbol, grammar)) {
                                // Next symbol is terminal
                                size_t beforeSize = follow[B].size();
                                follow[B].insert(nextSymbol);
                                if (follow[B].size() != beforeSize) {
                                    changed = true;
                                }
                                break;
                            } else {
                                // Next symbol is non-terminal
                                size_t beforeSize = follow[B].size();
                                
                                // Add FIRST(nextSymbol) - {ε} to FOLLOW(B)
                                for (const auto &f : first[nextSymbol]) {
                                    if (!isEpsilon(f)) {
                                        follow[B].insert(f);
                                    }
                                }
                                
                                if (follow[B].size() != beforeSize) {
                                    changed = true;
                                }
                                
                                // If nextSymbol cannot derive ε
                                if (first[nextSymbol].find("\\L") == first[nextSymbol].end()) {
                                    break;
                                }
                                
                                // If this is the last symbol
                                if (j == production.size() - 1) {
                                    beforeSize = follow[B].size();
                                    follow[B].insert(follow[A].begin(), follow[A].end());
                                    if (follow[B].size() != beforeSize) {
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (iteration >= MAX_ITERATIONS) {
        cerr << "Warning: FOLLOW calculation reached maximum iterations (" << MAX_ITERATIONS << ")\n";
    }
}

// ---------- Additional Helper Functions for Printing ----------

void printFirstSets(const map<string, set<string>> &first,
                   const map<string, vector<vector<string>>> &grammar) {
    cout << "\n=== FIRST Sets ===\n";
    for (const auto &entry : first) {
        // Show FIRST only for non-terminals
        if (grammar.find(entry.first) != grammar.end()) {
            cout << "FIRST(" << entry.first << ") = { ";
            bool firstItem = true;
            for (const string &symbol : entry.second) {
                if (!firstItem) cout << ", ";
                cout << symbol;
                firstItem = false;
            }
            cout << " }\n";
        }
    }
}

void printFollowSets(const map<string, set<string>> &follow) {
    cout << "\n=== FOLLOW Sets ===\n";
    for (const auto &entry : follow) {
        cout << "FOLLOW(" << entry.first << ") = { ";
        bool firstItem = true;
        for (const string &symbol : entry.second) {
            if (!firstItem) cout << ", ";
            cout << symbol;
            firstItem = false;
        }
        cout << " }\n";
    }
}

// Note: main() has been removed as it will be in a separate file
