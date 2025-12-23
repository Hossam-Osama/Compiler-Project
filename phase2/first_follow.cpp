#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
//#include "Read_File.h"
//#include "Mapping.h"
#include <string.h>

using namespace std;
bool compareStringToKeyByStrCmp(const map<string, set<string>>& myMap, const string& key) {
    // Iterate through the map and compare each key with the given string using strcmp
    for (const auto& entry : myMap) {
        if (strcmp(entry.first.c_str(), key.c_str()) == 0) {
            return true;  // Key found
        }
    }
    return false;  // Key not found
}

vector<string> split(const string &str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream stream(str);
    while (getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

vector<string> tokenize(const string &production) {
    vector<string> tokens;
    string token;
    istringstream stream(production);
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void calculateFirst(map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &rule : grammar) {
            const string &nonTerminal = rule.first;
            for (const auto &production : rule.second) {
                for (size_t i = 0; i < production.size(); ++i) {
                    const string &symbol = production[i];
                    
                    if (grammar.find(symbol) == grammar.end()) { // Terminal
                        if (first[nonTerminal].insert(symbol).second) {
                            changed = true;
                        }
                        break;
                    } else { // Non-terminal
                        size_t prevSize = first[nonTerminal].size();
                        first[nonTerminal].insert(first[symbol].begin(), first[symbol].end());
                        first[nonTerminal].erase("\\L");  // Remove epsilon if already included
                        
                        // If the symbol can derive epsilon, continue to the next symbol in production
                        if (first[symbol].find("\\L") == first[symbol].end()) {
                            break;
                        }
                        
                        // Otherwise, continue processing
                        if (first[nonTerminal].size() != prevSize) changed = true;
                    }
                }

                // Check if entire production can derive epsilon, then add epsilon to First set
                if (all_of(production.begin(), production.end(),
                           [&first](const string &sym) { return first[sym].count("\\L"); })) {
                    first[nonTerminal].insert("\\L");
                }
            }
        }
    }
}

void calculateFollow(map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first, map<string, set<string>> &follow, const string &startSymbol) {
    follow[startSymbol].insert("$");
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &rule : grammar) {
            const string &nonTerminal = rule.first;
            for (const auto &production : rule.second) {
                for (size_t i = 0; i < production.size(); ++i) {
                    const string &symbol = production[i];
                    if (grammar.find(symbol) != grammar.end()) { // Non-terminal
                        size_t prevSize = follow[symbol].size();
                        if (i + 1 < production.size()) {
                            for (size_t j = i + 1; j < production.size(); ++j) {
                                const string &nextSymbol = production[j];
                                if (grammar.find(nextSymbol) != grammar.end()) {
                                    follow[symbol].insert(first[nextSymbol].begin(), first[nextSymbol].end());
                                    follow[symbol].erase("\\L");
                                    if (first[nextSymbol].find("\\L") == first[nextSymbol].end()) break;
                                } else {
                                    follow[symbol].insert(nextSymbol);
                                    break;
                                }
                            }
                        }
                        if (i + 1 == production.size() || all_of(production.begin() + i + 1, production.end(),
                                                                 [&first](const string &sym) { return first[sym].count("\\L"); })) {
                            follow[symbol].insert(follow[nonTerminal].begin(), follow[nonTerminal].end());
                        }
                        if (follow[symbol].size() != prevSize) changed = true;
                    }
                }
            }
        }
    }
}

// int main() {
//     try {
//          map<string, vector<vector<string>>> grammar;
//         convertVectorToGrammar(processFile("./grammer.txt"),grammar);
//          for (const auto& outerPair : grammar) {
//         cout << "Part of Speech: " << outerPair.first << endl;
//         for (const auto& innerVector : outerPair.second) {
//             cout << "  Words: ";
//             for (const auto& str : innerVector) {
//                 cout << str << " ";
//             }
//             cout << endl;
//         }
//     }
//         // int numRules;
//         // cout << "Enter the number of grammar rules: ";
//         // cin >> numRules;
//         // cin.ignore();

//         // cout << "Enter the grammar rules (e.g., E = E + T | T):\n";
//         // for (int i = 0; i < numRules; ++i) {
//         //     string line;
//         //     getline(cin, line);
//         //     auto parts = split(line, '=');
//         //     string nonTerminal = parts[0];
//         //     auto productions = split(parts[1], '|');
//         //     for (const auto &prod : productions) {
//         //         grammar[nonTerminal].push_back(tokenize(prod));
//         //     }
//         // }

        
//         string startSymbol = "METHOD_BODY";
     
 
//         map<string, set<string>> first, follow;
//         calculateFirst(grammar, first);
//         calculateFollow(grammar, first, follow, startSymbol);

//         cout << "\nFirst sets:\n";
//         for (const auto &entry : first) {
//             cout << "First(" << entry.first << ") = { ";
//             for (const string &symbol : entry.second) {
//                 cout << symbol << " ";
//             }
//             cout << "}\n";
//         }

//         cout << "\nFollow sets:\n";
//         for (const auto &entry : follow) {
//             cout << "Follow(" << entry.first << ") = { ";
//             for (const string &symbol : entry.second) {
//                 cout << symbol << " ";
//             }
//             cout << "}\n";
//         }
//     } catch (const exception &e) {
//         cerr << e.what() << endl;
//         return 1;
//     }

//     return 0;
// }
