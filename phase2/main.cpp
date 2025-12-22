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
#include "CFGParser.cpp"

using namespace std;

// Forward declarations of functions defined in first_follow.cpp
string trim(const string &s);
string normalizeNonTerminal(const string &s);
vector<string> split(const string &str, char delimiter);
vector<string> tokenize(const string &production);
void calculateFirst(const map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first);
void calculateFollow(const map<string, vector<vector<string>>> &grammar, const map<string, set<string>> &first, map<string, set<string>> &follow, const string &startSymbol);
extern void run_phase1();

int main()
{
    run_phase1();

    try
    {
        map<string, vector<vector<string>>> grammar;
        string my_grammer_file = "my_grammer.txt";
        string first_follow_file = "first_follow_data.txt";
        string token_stream_file = "result.txt";
        string parsingTableFile = "parsing_table.txt";
        string testphase2_file = "test_phase2.txt";
        string cleaned_grammar_file = "cleaned_grammar.txt";
        string terminalsNonTerminalsFile = "terminal&non-terminal.txt";
        string ll1GrammarFile = "ll1_grammar.txt";

        CFGParser parser;
        parser.cleanGrammar(my_grammer_file, cleaned_grammar_file);
        parser.transformToLL1(cleaned_grammar_file, ll1GrammarFile);
        auto cfg_grammar = parser.parseGrammar(ll1GrammarFile);
        string startSymbol = parser.getStartSymbolName();

        // Convert CFGParser grammar to the expected format
        for (auto &pair : cfg_grammar)
        {
            if (pair.second->getName()[0] != '\'')
            { // Non-terminal
                NonTerminal *nt = dynamic_cast<NonTerminal *>(pair.second);
                if (nt)
                {
                    vector<vector<string>> prods;
                    for (auto &prod : nt->getProductions())
                    {
                        vector<string> p;
                        for (Item *item : prod)
                        {
                            string name = item->getName();
                            if (name[0] == '\'')
                            {
                                name = name.substr(1);
                            }
                            p.push_back(name);
                        }
                        prods.push_back(p);
                    }
                    grammar[pair.first] = prods;
                }
            }
        }

        if (grammar.empty())
        {
            throw runtime_error("No grammar rules provided.");
        }

        cout << "Start symbol: " + startSymbol + "\n";

        map<string, set<string>> first, follow;
        calculateFirst(grammar, first);
        calculateFollow(grammar, first, follow, startSymbol);

        std::ofstream firstFollowFile(first_follow_file);
        if (firstFollowFile)
        {
            for (const auto &entry : first)
            {
                firstFollowFile << "First(" << entry.first << ") = { ";
                for (auto sym : entry.second)
                {
                    if (!sym.empty() && sym.back() == '\'')
                        sym.pop_back();
                    firstFollowFile << "'" << sym << "' ";
                }
                firstFollowFile << "}\n";
            }

            for (const auto &entry : follow)
            {
                firstFollowFile << "Follow(" << entry.first << ") = { ";
                for (auto sym : entry.second)
                {
                    if (!sym.empty() && sym.back() == '\'')
                        sym.pop_back();
                    firstFollowFile << "'" << sym << "' ";
                }
                firstFollowFile << "}\n";
            }
        }
        firstFollowFile.close();
        cout << "First and Follow sets written to '" << first_follow_file << "'." << endl;

        // Epsilone is represented as \L in the files
        LL1ParsingTableGenerator parsingTableGenerator;
        parsingTableGenerator.generate(
            first_follow_file,
            terminalsNonTerminalsFile,
            parsingTableFile);

        map<pair<string, string>, string> parsingTable = parsingTableGenerator.getParsingTable();
        for (const auto &entry : parsingTable)
        {
            cout << entry.first.first << " -> " << entry.first.second << " -> " << entry.second << endl;
        }

        // mohamed code
        // start here use parsingTableFile ,testphase2_file , parsingTable
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
