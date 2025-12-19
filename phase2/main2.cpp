#include <iostream>
#include <map>
#include <string>

class Item {
public:
    virtual ~Item() = default;
    virtual std::string getName() const = 0;
    virtual bool isEqual(Item* item) const = 0;
};

class CFGParser {
public:
    void cleanGrammar(const std::string& filename, const std::string& cleanFilename);
    std::map<std::string, Item*> parseGrammar(const std::string& filename);
    bool debugParser(const std::string& filename);
    std::string getStartSymbolName();
    std::map<std::string, Item*> getGrammar();
private:
    std::string startSymbolName;
    std::map<std::string, Item*> CFG;
};

int main() {
    CFGParser parser;
    parser.debugParser("my_grammer.txt"); 
    return 0;
}
