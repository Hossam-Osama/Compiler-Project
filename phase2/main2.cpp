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
#include "../headers/CFGParser.h"
#include "../headers/first_follow.h"
#include "ProductionRules.h"

using namespace std;

// Forward declarations of functions defined in first_follow.cpp
string trim(const string &s);
string normalizeNonTerminal(const string &s);
vector<string> split(const string &str, char delimiter);
vector<string> tokenize(const string &production);
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
        string testphase2_file = "test.txt";
        string cleaned_grammar_file = "cleaned_grammar.txt";
        string terminalsNonTerminalsFile = "terminal&non-terminal.txt";
        string ll1GrammarFile = "ll1_grammar.txt";

        CFGParser parser;
        parser.cleanGrammar(my_grammer_file, cleaned_grammar_file);
        parser.transformToLL1(cleaned_grammar_file, ll1GrammarFile);
        auto cfg_grammar = parser.parseGrammar(ll1GrammarFile);
        string startSymbol = parser.getStartSymbolName();
        First_Follow p(cfg_grammar);
        p.createFirstSet();
        p.createFollowSet(startSymbol);
        p.printFirst();
        p.printFollow();
        p.printFirstAndFollowToFile(first_follow_file);

        // Epsilone is represented as \L in the files
        LL1ParsingTableGenerator parsingTableGenerator;
        parsingTableGenerator.generate(
            first_follow_file,
            terminalsNonTerminalsFile,
            parsingTableFile);

        // start here use parsingTableFile ,testphase2_file , parsingTable

        vector<string> tokens = readLexicalTokens();
        vector<string> productionRules = getProductionRules(tokens, startSymbol, parsingTableGenerator);
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }


    return 0;
}
