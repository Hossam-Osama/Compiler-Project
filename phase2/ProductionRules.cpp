#include<iostream>
#include <fstream>
#include<string>
#include<sstream>
#include <vector>
#include <stdexcept>
#include <map>
#include "../headers/LL1ParsingTableGenerator.h"

using namespace std;

vector<string> splitRulesString(const string& str, const string& delimiter) {
    vector<string> result;
    istringstream iss(str);
    string token;

    while (getline(iss, token, delimiter[0])) {
        result.push_back(token);
    }

    return result;
}
vector<string> getProductionRules(vector<string> input, string startRule, map<pair<string, string>, string> parsingTable) {
    vector<string> stack;
    vector<string> rules; // output

    stack.push_back("$");
    stack.push_back(startRule);

    while(!input.empty()) {
        string top = stack.back();
        string currentInput = input.front();

        cout << endl;
        if(currentInput != "$"){
        currentInput = "'" + currentInput + "'";
        }

        cout << "Stack top: " << top << ", Current input: " << currentInput << endl;
        if(top == currentInput && top == "$") {
            // Successfully parsed
            break;
        } else if(top == currentInput) {
            stack.pop_back();
            input.erase(input.begin());
        } else if(parsingTable.find({top, currentInput}) != parsingTable.end()) {
            // Apply production rule
            string fullRule = parsingTable[{top, currentInput}];

            // 2. Find the position of " = "
            size_t eqPos = fullRule.find(" = ");

            if (eqPos == string::npos) {
                cerr << "Error: Invalid production rule format: " << fullRule << endl;
                break;
            }

            string production = "";
            if (eqPos  != 0) {
                production = fullRule.substr(eqPos + 3);
            }


            cout << "\t ==> Applying production: " << top << " = " << production << endl;
            rules.push_back(production);

            stack.pop_back();

            if(production != "\\L") {
                vector<string> symbols;
                size_t pos = 0;
                while((pos = production.find(' ')) != string::npos) {
                    symbols.push_back(production.substr(0, pos));
                    production.erase(0, pos + 1);
                }
                symbols.push_back(production); // last symbol

                for(auto it = symbols.rbegin(); it != symbols.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        } else {
            // Error in parsing
            cerr << "Error: No rule for (" << top << ", " << currentInput << ")" << endl;
            break;
        }
    }


    return rules;
}

vector<string> readLexicalTokens() {
    ifstream inputFile("result.txt");

    if (!inputFile.is_open())
        throw runtime_error("Error: Could not open the file.");

    vector<string> tokens;
    string line;
    while (inputFile >> line){
        if (line == "assign") {
            tokens.push_back("=");
            continue;
        }
        tokens.push_back(line);
    }

    tokens.push_back("$");
    inputFile.close();

    cout << "Lexical tokens: " << endl;
    for (const auto& token : tokens) {
        cout << token << endl;
    }
    cout << endl;

    return tokens;
}