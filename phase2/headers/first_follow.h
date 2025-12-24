#ifndef PARSERTABLE_H
#define PARSERTABLE_H

#define LAMBDA "'\\L'"
#define END_SYMBOL "$"
#define SYNCH "SYNCH"

#include <map>
#include <set>
#include <string>
#include <iostream>

#include "Item.h"
#include "Terminal.h"
#include "NonTerminal.h"

class First_Follow {
public:
    // Constructor that initializes the grammar
    First_Follow(std::map<std::string, Item*> grammar);
    void createFirstSet();
    void createFollowSet(std::string startSymbol);
    void printFirst();
    void printFollow();
    void printFirstAndFollowToFile(const std::string& filename);
private:
    const std::map<std::string, Item*> grammar;
    std::map<std::string, std::set<std::string>> first;
    std::map<std::string, std::set<std::string>> follow;
    static bool addToSet(std::set<std::string>& targetSet, const std::set<std::string>& sourceSet);
    void setFirstSet(const std::string& curr);
    bool handleGrammarInFollowSet();
    bool getFollowSetOfNonTerminalUsingProduction(const std::string& currentProductionOwnerNonTerminal);
    [[nodiscard]] bool isTerminal(const std::string& itemName) const {
        auto it = grammar.find(itemName);
        if (it == grammar.end()) return true; // Assume special terminals like LAMBDA, END_SYMBOL are terminals
        return dynamic_cast<Terminal*>(it->second) != nullptr;
    }
};

#endif 
