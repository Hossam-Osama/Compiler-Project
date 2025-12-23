#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
using namespace std;
#include "Mapping.h"
// Function to split a string by spaces
vector<string> split(const string &str) {
    vector<string> tokens;
    istringstream stream(str);
    string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to convert a vector of vectors to the desired map structure
void convertVectorToGrammar(const vector<vector<string>> &input, map<string, vector<vector<string>>> &grammar) {
    for (const auto &row : input) {
        if (row.empty()) continue; // Skip empty rows
        string key = row[0]; // First element is the key
        vector<vector<string>> productions;

        // Process the remaining elements in the row
        for (size_t j = 1; j < row.size(); ++j) {
            productions.push_back(split(row[j])); // Split each production into a vector
        }

        // Add the key and its productions to the grammar map
        grammar[key] = productions;
    }
}

// // Main function to demonstrate usage
// int main() {
//     // Example vector of vectors (first element in each sub-vector is key, rest are productions)
//     vector<vector<string>> input = {
//         {"METHOD_BODY", "STATEMENT_LIST"},
//         {"STATEMENT_LIST", "STATEMENT", "STATEMENT_LIST STATEMENT"},
//         {"STATEMENT", "DECLARATION", "IF", "WHILE", "ASSIGNMENT"},
//         {"DECLARATION", "PRIMITIVE_TYPE 'id' ';'"},
//         {"PRIMITIVE_TYPE", "'int'", "'float'"},
//         {"IF", "'if' '(' EXPRESSION ')' '{' STATEMENT '}' 'else' '{' STATEMENT '}'"},
//         {"WHILE", "'while' '(' EXPRESSION ')' '{' STATEMENT '}'"},
//         {"ASSIGNMENT", "'id' '=' EXPRESSION ';'"},
//         {"EXPRESSION", "SIMPLE_EXPRESSION", "SIMPLE_EXPRESSION 'relop' SIMPLE_EXPRESSION"},
//         {"SIMPLE_EXPRESSION", "TERM", "SIGN TERM", "SIMPLE_EXPRESSION 'addop' TERM"},
//         {"TERM", "FACTOR", "TERM 'mulop' FACTOR"},
//         {"FACTOR", "'id'", "'num'", "'(' EXPRESSION ')'"},
//         {"SIGN", "'+'", "'-'"}
//     };

//     map<string, vector<vector<string>>> grammar;

//     // Convert the vector to the grammar map
//     convertVectorToGrammar(input, grammar);

//     // Print the resulting grammar map
//     for (const auto &rule : grammar) {
//         cout << rule.first << " = ";
//         for (const auto &production : rule.second) {
//             for (const auto &symbol : production) {
//                 cout << symbol << " ";
//             }
//             cout << "| ";
//         }
//         cout << endl;
//     }

//     return 0;
// }



 
