#ifndef CFGPARSER_H
#define CFGPARSER_H

#include "Item.h"
#include <map>
#include <vector>
#include <string>

class CFGParser {
public:
    // Clean the CFG file and expand rules with brackets
    void cleanGrammar(const std::string& filename, const std::string& cleanFilename);

    // Parses a context-free grammar from a file and constructs a map of items
    std::map<std::string, Item*> parseGrammar(const std::string& filename);

    // A function to debug the parser
    bool debugParser(const std::string& filename);

    // A function to get the start symbol
    std::string getStartSymbolName();
    
    void transformToLL1(const std::string &inputFilename, const std::string &outputFilename);

    // A function to get the grammar
    std::map<std::string, Item*> getGrammar();
    
private:
    std::string startSymbolName;
    std::map<std::string, Item*> CFG;
    bool startsWithPrefix(const std::vector<std::string> &production, const std::vector<std::string> &prefix);
    std::vector<std::string> tokenizeProduction(const std::string &production);
    void eliminateLeftRecursion(std::map<std::string, std::vector<std::vector<std::string>>> &grammarRules, std::vector<std::string> &ruleOrder);
    void performLeftFactoring(std::map<std::string, std::vector<std::vector<std::string>>> &grammarRules, std::vector<std::string> &ruleOrder);
    std::vector<std::string> findCommonPrefix(const std::vector<std::string> &prod1, const std::vector<std::string> &prod2);
};

#endif // CFGPARSER_H
