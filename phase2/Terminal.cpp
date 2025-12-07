#include <string>

class Item {
public:
    virtual ~Item() = default;
    virtual std::string getName() const = 0;
    virtual bool isEqual(Item* item) const = 0;
};

class Terminal : public Item {
public:
    Terminal(const std::string& name);
    std::string getName() const override;
    bool isEqual(Item* item) const override;
private:
    std::string name;
};

/**
 * @brief Constructs a Terminal object with the given name.
 *
 * @param name The name of the terminal.
 */
Terminal::Terminal(const std::string& name) : name(name) {}

/**
 * @brief Returns the name of the terminal.
 *
 * @return The name of the terminal.
 */
std::string Terminal::getName() const {
    return name;
}

/**
 * @brief Checks if the given item is equal to this terminal.
 *
 * @param item The item to compare with.
 * @return True if the given item is equal to this terminal, false otherwise.
 */
bool Terminal::isEqual(Item* item) const {
    return item->getName() == name;
}
