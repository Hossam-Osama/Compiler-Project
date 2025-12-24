#ifndef NONTERMINAL_H
#define NONTERMINAL_H

#include "Item.h"
#include <vector>

class NonTerminal : public Item {
public:
    // Constructor that takes the name of the non-terminal
    NonTerminal(const std::string& name);
    
    // Returns the name of the non-terminal
    std::string getName() const override;
    
    // Adds a production to the non-terminal
    void addProduction(const std::vector<Item*>& production);

    // Returns the productions of the non-terminal
    const std::vector<std::vector<Item*>>& getProductions();
    bool isEqual(Item* item) const override;

private:
    std::string name;
    std::vector<std::vector<Item*>> productions;
};

#endif // NONTERMINAL_H
