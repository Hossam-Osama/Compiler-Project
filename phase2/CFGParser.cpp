#include <string>
#include <vector>

class Item {
public:
    virtual ~Item() = default;
    virtual std::string getName() const = 0;
    virtual bool isEqual(Item* item) const = 0;
};

class NonTerminal : public Item {
public:
    NonTerminal(const std::string& name);
    std::string getName() const override;
    void addProduction(const std::vector<Item*>& production);
    const std::vector<std::vector<Item*>>& getProductions();
    bool isEqual(Item* item) const override;
private:
    std::string name;
    std::vector<std::vector<Item*>> productions;
};

class Terminal : public Item {
public:
    Terminal(const std::string& name);
    std::string getName() const override;
    bool isEqual(Item* item) const override;
private:
    std::string name;
};

#include <iostream>
#include <map>
#include <set>
#include <vector>

#define EPSILON '\0'

using namespace std;
class State {
 public:
  State();
  virtual ~State();
  bool is_accepted = false;
  bool is_invalid = false;
  map<char, vector<State*>> transitions;
  string accepted_rule;

  void add_transition(char action, State* to_state);
  void print_recursive(set<const State*>& visited) const;
  void print_state_info() const;
};

class StringProcessor {
 public:
  StringProcessor();
  virtual ~StringProcessor();
  vector<string> read_rules(string address);
  void skip_unnecessary_spaces(int& i, string rule_definition);
  string trim(const string& str);
  vector<string> string_processor(const string& input);
  string remove_backslash(string org);
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
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>



/**
 * @brief Cleans the CFG file and expands rules with nested brackets.
 *
 * Clean the CFG by removing all # symbols, putting each rule on a new line, 
 * and expanding rules with brackets into new nonterminals, while handling special cases like terminal brackets inside quotes.
 *
 * Additionally, checks for rule ambiguity and prints warnings for the following cases:
 * - A rule has no RHS.
 * - A rule has no LHS.
 * - A rule has no `=` (excluding when `=` is a terminal).
 *
 * @param filename The name of the file containing the context-free grammar.
 * @param cleanFilename The name of the file to write the cleaned grammar to.
 */
void CFGParser::cleanGrammar(const std::string& filename, const std::string& cleanFilename) {
    StringProcessor processor;
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::ostringstream cleanedContent;
    std::string line, fullRule;
    bool insideQuotes = false;  // Track whether we are inside quotes

    while (std::getline(inputFile, line)) {
        line = processor.trim(line);

        if (line.empty()) {
            continue;
        }

        for (char c : line) {
            if (c == '\'') {
                insideQuotes = !insideQuotes;
                fullRule += c;
            } else if (c == '#') {
                if (!insideQuotes) {
                    // If not inside quotes, this is a new rule
                    if (!fullRule.empty()) {
                        cleanedContent << fullRule << std::endl;
                        fullRule.clear();  // Clear fullRule for the next line
                    }
                } else {
                    fullRule += c;  // If inside quotes, treat # as part of the string
                }
            } else {
                fullRule += c;
            }
        }
        if (!fullRule.empty()) {
            fullRule += ' ';
        }
    }

    if (!fullRule.empty()) {
        cleanedContent << fullRule << std::endl;  // Add any remaining rule
    }

    inputFile.close();

    // Process expanded rules with brackets
    std::istringstream processedInput(cleanedContent.str());
    std::ostringstream expandedContent;
    std::string rule;
    int bufferCounter = 1; // Counter for new nonterminals
    bool isFirstRule = true;

    while (std::getline(processedInput, rule)) {
        size_t equalPos = rule.find('=');
        bool lhsEmpty = false;
        bool rhsEmpty = false;
        bool hasEqualSign = false;
        insideQuotes = false;

        // Check for ambiguity in the rule
        if (!rule.empty()) {
            for (size_t i = 0; i < rule.size(); ++i) {
                if (rule[i] == '\'') {
                    insideQuotes = !insideQuotes;
                } else if (!insideQuotes && rule[i] == '=') {
                    hasEqualSign = true;
                }
            }
        }

        if (!hasEqualSign) {
            std::cerr << "\033[33mWarning: Rule has no `=`: \033[0m" << rule << std::endl;
        } else {
            std::string lhs = processor.trim(rule.substr(0, equalPos));
            std::string rhs = processor.trim(rule.substr(equalPos + 1));

            if (lhs.empty()) {
                lhsEmpty = true;
                std::cerr << "\033[33mWarning: Rule has no LHS: \033[0m" << rule << std::endl;
            }

            if (rhs.empty()) {
                rhsEmpty = true;
                std::cerr << "\033[33mWarning: Rule has no RHS: \033[0m" << rule << std::endl;
            }

            if (!lhsEmpty && !rhsEmpty) {
                if (isFirstRule) {
                    startSymbolName = lhs;
                    isFirstRule = false;
                }
                std::stack<size_t> bracketStack;
                std::vector<std::string> newRules;

                for (size_t i = 0; i < rhs.size(); ++i) {
                    char c = rhs[i];

                    if (c == '\'') {
                        insideQuotes = !insideQuotes;
                    }

                    if (!insideQuotes) {
                        if (c == '(') {
                            bracketStack.push(i);
                        } else if (c == ')') {
                            if (!bracketStack.empty()) {
                                size_t openIndex = bracketStack.top();
                                bracketStack.pop();

                                // Extract the content inside the brackets
                                std::string innerContent = rhs.substr(openIndex + 1, i - openIndex - 1);
                                std::string bufferName = "BUFFER" + std::to_string(bufferCounter++);

                                // Replace the brackets with the new nonterminal
                                rhs.replace(openIndex, i - openIndex + 1, bufferName);

                                // Adjust the iterator position to account for replacement length
                                i = openIndex + bufferName.size() - 1;

                                // Add the new rule to the list
                                newRules.push_back(bufferName + " = " + innerContent);
                            }
                        }
                    }
                }

                // Write the processed rule and the newly generated rules
                expandedContent << lhs << " = " << rhs << std::endl;
                for (const std::string& newRule : newRules) {
                    expandedContent << newRule << std::endl;
                }
            }
        }
    }

    // Write the expanded content to the output file
    std::ofstream outputFile(cleanFilename);
    outputFile << expandedContent.str();
}



/**
 * @brief Parses a context-free grammar from a file and constructs a map of items.
 *
 * This function reads a context-free grammar from the specified file, constructs
 * terminal and non-terminal items, and stores them in a map where the key is the
 * token's name and the value is the corresponding item.
 *
 * @param cleanFilename The name of the file containing the cleaned context-free grammar.
 * @return A map where the key is the token's name and the value is the corresponding item.
 */
std::map<std::string, Item*> CFGParser::parseGrammar(const std::string& cleanFilename) {
    std::map<std::string, Item*> grammar;
    std::ifstream inputFile(cleanFilename);
    string outputFilename = "terminal&non-terminal.txt";
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << cleanFilename << std::endl;
        return grammar;
    }

    StringProcessor processor;
    std::string line;

    // Open output file
    std::ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFilename << std::endl;
        return grammar;
    }

    while (std::getline(inputFile, line)) {
        line = processor.trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;
        token = processor.trim(token);
        if (token.empty()) continue;

        if (grammar.find(token) == grammar.end()) {
            grammar[token] = new NonTerminal(token);
        }

        NonTerminal* nonTerminal = dynamic_cast<NonTerminal*>(grammar[token]);
        std::vector<Item*> production;
        bool passEqualSign = false;

        while (iss >> token) {
            token = processor.trim(token);
            if (token[0] == '=' && !passEqualSign) {
                passEqualSign = true;
                token = token.substr(1);
                if (token.empty()) continue;
            }
            if (token == "|") {
                nonTerminal->addProduction(production);
                production.clear();
            } else {
                if (token[0] == '\'') {  // Terminal
                    if (grammar.find(token) == grammar.end()) {
                        grammar[token] = new Terminal(token);
                    }
                    production.push_back(grammar[token]);
                } else {  // Non-terminal
                    if (grammar.find(token) == grammar.end()) {
                        grammar[token] = new NonTerminal(token);
                    }
                    production.push_back(grammar[token]);
                }
            }
        }
        if (!production.empty()) {
            nonTerminal->addProduction(production);
        }
    }

    // Write parsed grammar to output file
    for (const auto& pair : grammar) {
        outFile << "Item: " << pair.first;
        if (pair.second->getName()[0] == '\'') {
            outFile << " (Terminal)\n";
        } else {
            outFile << " (Non-terminal)\n";
            NonTerminal* nt = dynamic_cast<NonTerminal*>(pair.second);
            auto productions = nt->getProductions();
            for (const auto& prod : productions) {
                outFile << "  Production: ";
                for (Item* item : prod) {
                    outFile << item->getName() << " ";
                }
                outFile << "\n";
            }
        }
        outFile << "\n";
    }

    outFile.close();
    CFG = grammar;
    return grammar;
}

/**
 * @brief A function to debug the parser.
 * 
 * This function is used for debugging the parser by parsing a context-free grammar from 
 * a file and printing the names of the items in the grammar and their productions.
 * 
 * @param filename The name of the file containing the context-free grammar.
 * @return True if the parser finished parsing without compilation errors.
 */
bool CFGParser::debugParser(const std::string& filename) {
    CFGParser parser;

    // Clean the grammar file
    parser.cleanGrammar(filename, "cleaned_grammar.txt");

    parser.parseGrammar("cleaned_grammar.txt");

    std::map<std::string, Item*> grammar = parser.getGrammar();

    std::cout << "************************************" << std::endl;

    std::cout << "Start symbol: " << parser.getStartSymbolName() << std::endl;
    std::vector<std::vector<Item*>> startSymbolProductions = dynamic_cast<NonTerminal*>(grammar[parser.getStartSymbolName()])->getProductions();
    std::cout << "Start symbol productions: " << std::endl;
    for (const auto& production : startSymbolProductions) {
        for (Item* item : production) {
            std::cout << item->getName() << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "************************************" << std::endl;

    for (const auto& pair : grammar) {
        std::cout << "Item: " << pair.first << std::endl;
        if (pair.second->getName()[0] == '\'') {
            std::cout << "Terminal" << std::endl;
            continue;
        } else {
            std::cout << "Non-terminal" << std::endl;
        }
        std::vector<std::vector<Item*>> productions = dynamic_cast<NonTerminal*>(pair.second)->getProductions();
        for (const auto& production : productions) {
            std::cout << "Production: ";
            for (Item* item : production) {
                std::cout << item->getName() << " ";
            }
            std::cout << std::endl;
        }
    }

    return true;
}

/**
 * @brief A function to get the start symbol.
 * 
 * This function is used to get the start symbol of the grammar.
 * Assumption: The start symbol is the first non-terminal in the grammar.
 * 
 * @return The start symbol of the grammar.
 */
std::string CFGParser::getStartSymbolName() {
    if (startSymbolName.empty()) {
        std::cerr << "\033[31mError: Start symbol is empty. Make sure you parse the grammar first.\033[0m" << std::endl;
    }
    return startSymbolName;
}

/**
 * @brief A function to get the grammar.
 * 
 * This function is used to get the grammar.
 * 
 * @return The grammar.
 */
std::map<std::string, Item*> CFGParser::getGrammar() {
    if (CFG.empty()) {
        std::cerr << "\033[31mError: Grammar is empty. Make sure you parse the grammar first.\033[0m" << std::endl;
    }
    return CFG;
}