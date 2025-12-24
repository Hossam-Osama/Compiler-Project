#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
public:
    virtual ~Item() = default;

    // Returns the name of the item
    virtual std::string getName() const = 0;
    virtual bool isEqual(Item* item) const = 0;
};

#endif // ITEM_H
