#include "../headers/NonTerminal.h"

/**
 * @brief Constructs a NonTerminal object with the given name.
 *
 * @param name The name of the non-terminal.
 */
NonTerminal::NonTerminal(const std::string& name) : name(name) {}

/**
 * @brief Returns the name of the non-terminal.
 *
 * @return The name of the non-terminal.
 */
std::string NonTerminal::getName() const {
    return name;
}

/**
 * @brief Adds a production to the non-terminal.
 *
 * @param production The production to be added.
 */
void NonTerminal::addProduction(const std::vector<Item*>& production) {
    productions.push_back(production);
}

/**
 * @brief Returns the productions of the non-terminal.
 *
 * @return The productions of the non-terminal.
 */
const std::vector<std::vector<Item*>>& NonTerminal::getProductions() {
    return productions;
}

/**
 * @brief Checks if the given item is equal to this non-terminal.
 *
 * @param item The item to compare with.
 * @return True if the given item is equal to this non-terminal, false otherwise.
 */
bool NonTerminal::isEqual(Item* item) const {
    return item->getName() == name;
}

