#include<iostream>
#include<string>
#include<sstream>
#include <vector>
#include <map>
#include "LL1ParsingTableGenerator.h"

using namespace std;

vector<string> splitString(const string& str, const string& delimiter) {
    vector<string> result;
    istringstream iss(str);
    string token;

    while (getline(iss, token, delimiter[0])) {
        result.push_back(token);
    }

    return result;
}
vector<string> getProductionRules(vector<string> input, map<pair<string, string>, string> parsingTable) {
    vector<string> stack;
    vector<string> rules; // output

    stack.push_back("$");

    while(!input.empty()) {
        string top = stack.back();
        string currentInput = input.front();

        if(top == currentInput && top == "$") {
            // Successfully parsed
            break;
        } else if(top == currentInput) {
            // Match and pop
            stack.pop_back();
            input.erase(input.begin());
        } else if(parsingTable.find({top, currentInput}) != parsingTable.end()) {
            // Apply production rule
            string production = parsingTable[{top, currentInput}];
            rules.push_back(production);

            production = splitString(production, "::= ")[1];

            stack.pop_back();
            if(production != "ε") {
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

