#include<iostream>
#include <fstream>
#include<string>
#include<sstream>
#include <vector>
#include <stdexcept>
#include <map>
#include "../headers/LL1ParsingTableGenerator.h"

using namespace std;

void printResults(const vector<string>& stack, string fileName) {
    // Construct the full filename
    string fullPath = fileName + ".txt";

    // 'ofstream' automatically creates the file if it doesn't exist.
    // If it does exist, it overwrites it.
    ofstream out(fullPath);

    // Check if the file was successfully created/opened
    if (!out.is_open()) {
        cerr << "Error: Failed to create or open file '" << fullPath << "'\n";
        return; // Stop the function so we don't crash trying to write
    }

    out << "LL(1) Parser Output\n";
    out << "Parsing completed successfully\n";

    // Optional: Write the stack/rules if needed
    for (const string& s : stack) {
        out << s << "\n";
    }

    out.close();
}

vector<string> splitRulesString(const string& str, const string& delimiter) {
    vector<string> result;
    istringstream iss(str);
    string token;

    while (getline(iss, token, delimiter[0])) {
        result.push_back(token);
    }

    return result;
}
vector<string> getProductionRules(vector<string>& input, string& startRule, LL1ParsingTableGenerator& parsingTableGenerator) {
    vector<string> stack;
    vector<string> rules; // output
    vector<string> stackTrace; // for debugging
    map<pair<string, string>, string> parsingTable = parsingTableGenerator.getParsingTable();
    map<string, set<string>> folowSet = parsingTableGenerator.getFollowSets();
    set<string> terminals = parsingTableGenerator.getTerminals();

    stack.push_back("$");
    stack.push_back(startRule);

    while(!input.empty()) {
        string top = stack.back();
        string currentInput = input.front();

        // Handle quote formatting for table lookup
        string lookupInput = currentInput;
        if(lookupInput != "$"){
            lookupInput = "'" + lookupInput + "'";
        }

        stackTrace.push_back("Stack top: " + top + ", Current input: " + lookupInput + "\n");

        string fullRule = parsingTable[{top, lookupInput}];
        // 1. Success Case
        if(top == lookupInput && top == "$") {
            break;
        }
        // 2. Match Case (Terminal on stack matches input)
        else if(top == lookupInput) {
            stack.pop_back();
            input.erase(input.begin());
        }
        // 3. Apply Rule Case (Non-terminal on stack, rule exists in table)
        else if(parsingTable.find({top, lookupInput}) != parsingTable.end() && fullRule !="EMPTY") {
            if (fullRule.empty()) {
                stackTrace.push_back("Error: Empty production rule found for " + top + " has no " + lookupInput + " production rule.\n");
                break; // Stop parsing on internal error
            }
            size_t eqPos = fullRule.find(" = ");

            // Internal Check: Is the grammar rule string valid?
            if (eqPos == string::npos) {
                stackTrace.push_back("Error: Invalid production rule format in table: " + fullRule + "\n");
                break; // Stop parsing on internal error
            }

            string production = "";
            if (eqPos != 0) {
                production = fullRule.substr(eqPos + 3);
            }

            stackTrace.push_back("\t ==>Applying production: " + top + " = " + production + "\n");
            rules.push_back(production);

            stack.pop_back(); // Pop the Non-Terminal

            // Push new symbols to stack (unless epsilon)
            if(production != "\\L") {
                vector<string> symbols;
                size_t pos = 0;
                while((pos = production.find(' ')) != string::npos) {
                    symbols.push_back(production.substr(0, pos));
                    production.erase(0, pos + 1);
                }
                symbols.push_back(production);

                for(auto it = symbols.rbegin(); it != symbols.rend(); ++it) {
                    stack.push_back(*it);
                }
            }
        }
        // 4. Error Case (No rule found) -> THIS IS WHERE PANIC MODE GOES
        else {
            // Case 1: Stack top is a terminal
            if(terminals.find(top) != terminals.end()) {
                stackTrace.push_back("*** SYNTAX ERROR: Unexpected token '" + currentInput + "' -- Expected '" + top + "'. ***\n");
                rules.push_back("Error: Unexpected token '" + currentInput +"' -- Expected '" + top + "'.");
                stackTrace.push_back("*** PANIC MODE: Discarding top of stack '" + top + "' ***\n");
                stack.pop_back();
                continue;
            }

            // Case 2: Stack top is a non-terminal
            else {
                // sync point
                if(folowSet.find(top) != folowSet.end() &&
                    folowSet[top].find(currentInput) != folowSet[top].end()) {
                    stackTrace.push_back("*** SYNTAX ERROR: Missing expected token for non-terminal " + top + ". ***\n");
                    rules.push_back("Error: Missing expected token for non-terminal " + top + ".");
                    stackTrace.push_back("*** PANIC MODE: Discarding non-terminal '" + top + "' from stack ***\n");
                    stack.pop_back();
                }
                else {
                    stackTrace.push_back("*** SYNTAX ERROR: Unexpected token '" + currentInput + "' - No rule for non-terminal '" + top + "'. ***\n");
                    rules.push_back("Error: Unexpected token '" + currentInput + "' - No rule for non-terminal '" + top + "'.");
                    stackTrace.push_back("*** PANIC MODE: Discarding input token '" + currentInput + "' ***\n");
                    input.erase(input.begin());
                }
            }
        }
    }

    printResults(rules, "output");
    printResults(stackTrace, "stack_trace");

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

    return tokens;
}