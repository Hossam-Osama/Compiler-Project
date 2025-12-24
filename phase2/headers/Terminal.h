#ifndef TERMINAL_H
#define TERMINAL_H

#include "Item.h"

class Terminal : public Item {
public:
    // Constructor that takes the name of the terminal
    Terminal(const std::string& name);
    
    // Returns the name of the terminal
    std::string getName() const override;
    bool isEqual(Item* item) const override;

private:
    std::string name;
};

#endif // TERMINAL_H
