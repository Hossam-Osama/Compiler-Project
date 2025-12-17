#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include "LL1ParsingTableGenerator.h"

using namespace std;

// Forward declarations of functions defined in first_follow.cpp
string trim(const string &s);
string normalizeNonTerminal(const string &s);
vector<string> split(const string &str, char delimiter);
vector<string> tokenize(const string &production);
void calculateFirst(const map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first);
void calculateFollow(const map<string, vector<vector<string>>> &grammar, const map<string, set<string>> &first, map<string, set<string>> &follow, const string &startSymbol);

int main()
{
    try
    {
        map<string, vector<vector<string>>> grammar;
        string my_grammer_file = "my_grammer.txt";
        string first_follow_file = "first_follow_data.txt";
        string token_stream_file = "../phase1/result.txt";
        string parsingTableFile = "parsing_table.txt";
        vector<string> lines;
        ifstream fin(my_grammer_file);
        if (fin)
        {
            string raw;
            while (getline(fin, raw))
            {
                // Remove anything after a '#' (treat as comment marker)
                size_t cpos = raw.find('#');
                if (cpos != string::npos)
                    raw.erase(cpos);

                // Trim whitespace
                size_t first = raw.find_first_not_of(" \t\r\n");
                if (first == string::npos)
                    continue;
                size_t last = raw.find_last_not_of(" \t\r\n");
                string line = raw.substr(first, last - first + 1);
                if (line.empty())
                    continue;
                lines.push_back(line);
            }
        }
        else
        {
            cout << "Failed to open grammar file '" << my_grammer_file << "'." << endl;
            return 1;
        }

        // Parse collected lines for rules containing '='
        for (const auto &line : lines)
        {
            auto parts = split(line, '=');
            if (parts.size() != 2)
            {
                continue; // ignore malformed or non-rule lines
            }
            string nonTerminal = parts[0];
            // Trim nonTerminal
            size_t a = nonTerminal.find_first_not_of(" \t\r\n");
            size_t b = nonTerminal.find_last_not_of(" \t\r\n");
            if (a == string::npos)
                continue;
            nonTerminal = nonTerminal.substr(a, b - a + 1);

            auto productions = split(parts[1], '|');
            for (const auto &prod : productions)
            {
                grammar[nonTerminal].push_back(tokenize(prod));
            }
        }

        if (grammar.empty())
        {
            throw runtime_error("No grammar rules provided.");
        }

        string startSymbol = grammar.begin()->first;

        map<string, set<string>> first, follow;
        calculateFirst(grammar, first);
        calculateFollow(grammar, first, follow, startSymbol);

        std::ofstream firstFollowFile(first_follow_file);
        if (firstFollowFile)
        {
            for (const auto &entry : first)
            {
                firstFollowFile << "First(" << entry.first << ") = { ";
                for (const auto &sym : entry.second)
                    firstFollowFile << sym << ' ';
                firstFollowFile << "}\n";
            }

            for (const auto &entry : follow)
            {
                firstFollowFile << "Follow(" << entry.first << ") = { ";
                for (const auto &sym : entry.second)
                    firstFollowFile << sym << ' ';
                firstFollowFile << "}\n";
            }
        }
        firstFollowFile.close();
        cout << "First and Follow sets written to '" << first_follow_file << "'." << endl;

        LL1ParsingTableGenerator parsingTableGenerator;
        parsingTableGenerator.generate(
            first_follow_file,
            my_grammer_file,
            parsingTableFile);

        map<pair<string, string>, string> parsingTable = parsingTableGenerator.getParsingTable();
        for (const auto &entry : parsingTable)
        {
            cout << entry.first.first << " -> " << entry.first.second << " -> " << entry.second << endl;
        }
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
