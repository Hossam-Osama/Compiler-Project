#include <string>
#include <vector>
#include <set>

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
    void transformToLL1(const std::string &inputFilename, const std::string &outputFilename);
    std::string getStartSymbolName();
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

        // Replace curly quotes with straight quotes
        size_t pos = 0;
        std::string left_quote = "‘";
        std::string right_quote = "’";
        while ((pos = line.find(left_quote, pos)) != std::string::npos) {
            line.replace(pos, left_quote.length(), "'");
            pos += 1; // move past the replacement
        }
        pos = 0;
        while ((pos = line.find(right_quote, pos)) != std::string::npos) {
            line.replace(pos, right_quote.length(), "'");
            pos += 1;
        }

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


// /**
//  * @brief Transforms a CFG into LL(1) form by eliminating left recursion and performing left factoring.
//  *
//  * @param inputFilename The cleaned grammar file to transform.
//  * @param outputFilename The output file for the LL(1) grammar.
//  */
// void CFGParser::transformToLL1(const std::string& inputFilename, const std::string& outputFilename) {
//     StringProcessor processor;
//     std::ifstream inputFile(inputFilename);
//     if (!inputFile.is_open()) {
//         std::cerr << "Error: Could not open file " << inputFilename << std::endl;
//         return;
//     }

//     // Store grammar rules as map: LHS -> vector of productions (each production is vector of tokens)
//     std::map<std::string, std::vector<std::vector<std::string>>> grammarRules;
//     std::vector<std::string> ruleOrder; // To preserve order of rules
//     std::string line;

//     // Parse the input grammar
//     while (std::getline(inputFile, line)) {
//         line = processor.trim(line);
//         if (line.empty()) continue;

//         size_t equalPos = line.find('=');
//         if (equalPos == std::string::npos) continue;

//         std::string lhs = processor.trim(line.substr(0, equalPos));
//         std::string rhs = processor.trim(line.substr(equalPos + 1));

//         if (grammarRules.find(lhs) == grammarRules.end()) {
//             ruleOrder.push_back(lhs);
//         }

//         // Split RHS by '|' and tokenize each production
//         std::vector<std::vector<std::string>> productions;
//         std::string currentProduction;
//         bool insideQuotes = false;

//         for (size_t i = 0; i < rhs.size(); ++i) {
//             char c = rhs[i];
//             if (c == '\'') {
//                 insideQuotes = !insideQuotes;
//                 currentProduction += c;
//             } else if (c == '|' && !insideQuotes) {
//                 if (!currentProduction.empty()) {
//                     productions.push_back(tokenizeProduction(processor.trim(currentProduction)));
//                     currentProduction.clear();
//                 }
//             } else {
//                 currentProduction += c;
//             }
//         }
//         if (!currentProduction.empty()) {
//             productions.push_back(tokenizeProduction(processor.trim(currentProduction)));
//         }

//         grammarRules[lhs] = productions;
//     }
//     inputFile.close();

//     // Step 1: Eliminate left recursion
//     std::cout << "Eliminating left recursion..." << std::endl;
//     eliminateLeftRecursion(grammarRules, ruleOrder);

//     // Step 2: Perform left factoring
//     std::cout << "Performing left factoring..." << std::endl;
//     performLeftFactoring(grammarRules, ruleOrder);

//     // Write the transformed grammar to output file
//     std::ofstream outputFile(outputFilename);
//     if (!outputFile.is_open()) {
//         std::cerr << "Error: Could not open output file " << outputFilename << std::endl;
//         return;
//     }

//     for (const std::string& lhs : ruleOrder) {
//         if (grammarRules.find(lhs) != grammarRules.end()) {
//             outputFile << lhs << " = ";
//             const auto& productions = grammarRules[lhs];
//             for (size_t i = 0; i < productions.size(); ++i) {
//                 for (size_t j = 0; j < productions[i].size(); ++j) {
//                     outputFile << productions[i][j];
//                     if (j < productions[i].size() - 1) outputFile << " ";
//                 }
//                 if (i < productions.size() - 1) outputFile << " | ";
//             }
//             outputFile << std::endl;
//         }
//     }
//     outputFile.close();
//     std::cout << "LL(1) grammar written to " << outputFilename << std::endl;
// }

/**
 * @brief Transforms a CFG into LL(1) form by eliminating ALL left recursion (immediate and non-immediate) 
 *        and performing left factoring.
 *
 * This implements the complete algorithm:
 * 1. Order non-terminals A₁, A₂, ..., Aₙ
 * 2. For i from 1 to n:
 *      For j from 1 to i-1:
 *        Replace each production Aᵢ → Aⱼγ by Aᵢ → α₁γ | α₂γ | ... | αₖγ
 *        where Aⱼ → α₁ | α₂ | ... | αₖ
 *      Eliminate immediate left recursion among Aᵢ productions
 * 3. Perform left factoring
 *
 * @param inputFilename The cleaned grammar file to transform.
 * @param outputFilename The output file for the LL(1) grammar.
 */
void CFGParser::transformToLL1(const std::string& inputFilename, const std::string& outputFilename) {
    StringProcessor processor;
    std::ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << inputFilename << std::endl;
        return;
    }

    // Store grammar rules as map: LHS -> vector of productions (each production is vector of tokens)
    std::map<std::string, std::vector<std::vector<std::string>>> grammarRules;
    std::vector<std::string> ruleOrder; // To preserve order of rules (A₁, A₂, ..., Aₙ)
    std::string line;

    // ==================== STEP 1: Parse the input grammar ====================
    while (std::getline(inputFile, line)) {
        line = processor.trim(line);
        if (line.empty()) continue;

        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;

        std::string lhs = processor.trim(line.substr(0, equalPos));
        std::string rhs = processor.trim(line.substr(equalPos + 1));

        if (grammarRules.find(lhs) == grammarRules.end()) {
            ruleOrder.push_back(lhs);
        }

        // Split RHS by '|' and tokenize each production
        std::vector<std::vector<std::string>> productions;
        std::string currentProduction;
        bool insideQuotes = false;

        for (size_t i = 0; i < rhs.size(); ++i) {
            char c = rhs[i];
            if (c == '\'') {
                insideQuotes = !insideQuotes;
                currentProduction += c;
            } else if (c == '|' && !insideQuotes) {
                if (!currentProduction.empty()) {
                    productions.push_back(tokenizeProduction(processor.trim(currentProduction)));
                    currentProduction.clear();
                }
            } else {
                currentProduction += c;
            }
        }
        if (!currentProduction.empty()) {
            productions.push_back(tokenizeProduction(processor.trim(currentProduction)));
        }

        grammarRules[lhs] = productions;
    }
    inputFile.close();

    // ==================== STEP 2: Eliminate ALL left recursion (general algorithm) ====================
    std::cout << "Eliminating all left recursion (immediate and non-immediate)..." << std::endl;
    
    int n = ruleOrder.size();
    
    // Main loop: for i from 1 to n
    for (int i = 0; i < n; ++i) {
        std::string Ai = ruleOrder[i];
        
        std::cout << "\n--- Processing " << Ai << " (position " << (i+1) << " of " << n << ") ---" << std::endl;
        
        // Inner loop: for j from 1 to i-1
        for (int j = 0; j < i; ++j) {
            std::string Aj = ruleOrder[j];
            
            std::cout << "  Checking for productions " << Ai << " → " << Aj << "γ..." << std::endl;
            
            // Check if Ai has any production that starts with Aj
            std::vector<std::vector<std::string>>& AiProductions = grammarRules[Ai];
            std::vector<std::vector<std::string>> newProductions;
            bool substitutionMade = false;
            
            for (const auto& production : AiProductions) {
                // Check if this production starts with Aj
                if (!production.empty() && production[0] == Aj) {
                    substitutionMade = true;
                    std::cout << "    Found: " << Ai << " → " << Aj;
                    for (size_t k = 1; k < production.size(); ++k) {
                        std::cout << " " << production[k];
                    }
                    std::cout << std::endl;
                    
                    // Extract γ (everything after Aj)
                    std::vector<std::string> gamma(production.begin() + 1, production.end());
                    
                    // Get all productions of Aj: Aj → α₁ | α₂ | ... | αₖ
                    const std::vector<std::vector<std::string>>& AjProductions = grammarRules[Aj];
                    
                    // Replace Ai → Ajγ with Ai → α₁γ | α₂γ | ... | αₖγ
                    for (const auto& alpha : AjProductions) {
                        std::vector<std::string> newProduction = alpha;
                        newProduction.insert(newProduction.end(), gamma.begin(), gamma.end());
                        newProductions.push_back(newProduction);
                        
                        std::cout << "    Substituted with: " << Ai << " → ";
                        for (const auto& token : newProduction) {
                            std::cout << token << " ";
                        }
                        std::cout << std::endl;
                    }
                } else {
                    // This production doesn't start with Aj, keep it as is
                    newProductions.push_back(production);
                }
            }
            
            // Update Ai's productions if we made any substitutions
            if (substitutionMade) {
                grammarRules[Ai] = newProductions;
                std::cout << "  Substitution completed for " << Ai << std::endl;
            }
        }
        
        // Now eliminate immediate left recursion among Ai productions
        std::cout << "  Eliminating immediate left recursion in " << Ai << "..." << std::endl;
        std::vector<std::string> tempOrder = {Ai};
        eliminateLeftRecursion(grammarRules, ruleOrder);
    }
    
    std::cout << "\nAll left recursion eliminated!" << std::endl;

    // ==================== STEP 3: Perform left factoring ====================
    std::cout << "\nPerforming left factoring..." << std::endl;
    performLeftFactoring(grammarRules, ruleOrder);

    // ==================== STEP 4: Write the transformed grammar to output file ====================
    std::ofstream outputFile(outputFilename);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputFilename << std::endl;
        return;
    }

    for (const std::string& lhs : ruleOrder) {
        if (grammarRules.find(lhs) != grammarRules.end()) {
            outputFile << lhs << " = ";
            const auto& productions = grammarRules[lhs];
            for (size_t i = 0; i < productions.size(); ++i) {
                for (size_t j = 0; j < productions[i].size(); ++j) {
                    outputFile << productions[i][j];
                    if (j < productions[i].size() - 1) outputFile << " ";
                }
                if (i < productions.size() - 1) outputFile << " | ";
            }
            outputFile << std::endl;
        }
    }
    outputFile.close();
    std::cout << "\n LL(1) grammar written to " << outputFilename << std::endl;
}

/**
 * @brief Tokenizes a production string into individual tokens.
 */
std::vector<std::string> CFGParser::tokenizeProduction(const std::string& production) {
    std::vector<std::string> tokens;
    std::string token;
    bool insideQuotes = false;

    for (size_t i = 0; i < production.size(); ++i) {
        char c = production[i];
        if (c == '\'') {
            if (insideQuotes) {
                token += c;
                tokens.push_back(token);
                token.clear();
                insideQuotes = false;
            } else {
                insideQuotes = true;
                token += c;
            }
        } else if (std::isspace(c) && !insideQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief Eliminates immediate left recursion from the grammar.
 */
void CFGParser::eliminateLeftRecursion(std::map<std::string, std::vector<std::vector<std::string>>>& grammarRules,
                                       std::vector<std::string>& ruleOrder) {
    std::vector<std::string> currentOrder = ruleOrder;
    
    for (const std::string& lhs : currentOrder) {
        if (grammarRules.find(lhs) == grammarRules.end()) continue;
        
        std::vector<std::vector<std::string>>& productions = grammarRules[lhs];
        std::vector<std::vector<std::string>> recursiveProds;
        std::vector<std::vector<std::string>> nonRecursiveProds;

        // Separate recursive and non-recursive productions
        for (const auto& prod : productions) {
            if (!prod.empty() && prod[0] == lhs) {
                recursiveProds.push_back(prod);
            } else {
                nonRecursiveProds.push_back(prod);
            }
        }

        // If there's left recursion, eliminate it
        if (!recursiveProds.empty()) {
            std::string newNonTerminal = lhs + "'";
            std::vector<std::vector<std::string>> newProductions;
            std::vector<std::vector<std::string>> primeProductions;

            // A -> βA' for all non-recursive productions β
            for (const auto& beta : nonRecursiveProds) {
                std::vector<std::string> newProd = beta;
                newProd.push_back(newNonTerminal);
                newProductions.push_back(newProd);
            }

            // A' -> αA' | ε for all recursive productions Aα
            for (const auto& alphaProd : recursiveProds) {
                std::vector<std::string> alpha(alphaProd.begin() + 1, alphaProd.end());
                alpha.push_back(newNonTerminal);
                primeProductions.push_back(alpha);
            }
            // Add epsilon production
            primeProductions.push_back(std::vector<std::string>{"ε"});

            grammarRules[lhs] = newProductions;
            grammarRules[newNonTerminal] = primeProductions;
            ruleOrder.push_back(newNonTerminal);

            std::cout << "  Eliminated immediate left recursion in " << lhs << std::endl;
        }
    }
}

/**
 * @brief Performs left factoring on the grammar.
 */
void CFGParser::performLeftFactoring(std::map<std::string, std::vector<std::vector<std::string>>>& grammarRules,
                                     std::vector<std::string>& ruleOrder) {
    bool changed = true;
    int primeCount = 1;

    while (changed) {
        changed = false;
        std::vector<std::string> currentOrder = ruleOrder;

        for (const std::string& lhs : currentOrder) {
            if (grammarRules.find(lhs) == grammarRules.end()) continue;

            std::vector<std::vector<std::string>>& productions = grammarRules[lhs];
            
            // Find common prefixes
            for (size_t i = 0; i < productions.size(); ++i) {
                for (size_t j = i + 1; j < productions.size(); ++j) {
                    std::vector<std::string> commonPrefix = findCommonPrefix(productions[i], productions[j]);
                    
                    if (!commonPrefix.empty()) {
                        // Found common prefix, perform left factoring
                        std::string newNonTerminal = lhs + std::to_string(primeCount++);
                        
                        // Collect all productions with this common prefix
                        std::vector<size_t> indicesToRemove;
                        std::vector<std::vector<std::string>> factoredProductions;
                        
                        for (size_t k = 0; k < productions.size(); ++k) {
                            if (startsWithPrefix(productions[k], commonPrefix)) {
                                indicesToRemove.push_back(k);
                                std::vector<std::string> suffix(productions[k].begin() + commonPrefix.size(), 
                                                               productions[k].end());
                                if (suffix.empty()) {
                                    suffix.push_back("ε");
                                }
                                factoredProductions.push_back(suffix);
                            }
                        }

                        // Remove old productions in reverse order
                        for (auto it = indicesToRemove.rbegin(); it != indicesToRemove.rend(); ++it) {
                            productions.erase(productions.begin() + *it);
                        }

                        // Add new factored production
                        std::vector<std::string> newProd = commonPrefix;
                        newProd.push_back(newNonTerminal);
                        productions.push_back(newProd);

                        // Add new non-terminal rule
                        grammarRules[newNonTerminal] = factoredProductions;
                        ruleOrder.push_back(newNonTerminal);

                        std::cout << "  Left factored " << lhs << " (common prefix of length " 
                                  << commonPrefix.size() << ")" << std::endl;
                        
                        changed = true;
                        break;
                    }
                }
                if (changed) break;
            }
            if (changed) break;
        }
    }
}

/**
 * @brief Finds the common prefix between two productions.
 */
std::vector<std::string> CFGParser::findCommonPrefix(const std::vector<std::string>& prod1,
                                                     const std::vector<std::string>& prod2) {
    std::vector<std::string> prefix;
    size_t minLen = std::min(prod1.size(), prod2.size());
    
    for (size_t i = 0; i < minLen; ++i) {
        if (prod1[i] == prod2[i]) {
            prefix.push_back(prod1[i]);
        } else {
            break;
        }
    }
    return prefix;
}

/**
 * @brief Checks if a production starts with a given prefix.
 */
bool CFGParser::startsWithPrefix(const std::vector<std::string>& production,
                                const std::vector<std::string>& prefix) {
    if (prefix.size() > production.size()) return false;
    
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (production[i] != prefix[i]) return false;
    }
    return true;
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

    // Transform to LL(1) form
    parser.transformToLL1("cleaned_grammar.txt", "ll1_grammar.txt");

    // Parse the cleaned grammar file
    parser.parseGrammar("ll1_grammar.txt");

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


// State implementations
State::State() {
  // ctor
}

State::~State() {
  // Clear transitions to avoid dangling pointers
  for (auto& transition : transitions) {
    transition.second.clear();  // Clear vector of pointers
  }
  transitions.clear();  // Clear the map itself
}

void State::add_transition(char a, State* state) {
  if (transitions.find(a) == transitions.end()) {
    vector<State*> v;
    transitions[a] = v;
  }
  transitions[a].push_back(state);
}

void State::print_recursive(set<const State*>& visited) const {
  cout << this << endl;
  if (visited.count(this)) {
    cout << "(already visited)" << endl;
    return;
  }

  visited.insert(this);

  cout << "Is Accepted: " << (is_accepted ? "Yes" : "No") << endl;
  cout << "Transitions:";

  for (const auto& [action, states] : transitions) {
    cout << "     Action: " << action << " -> ";
    for (const auto& state : states) {
      cout << state << " ";
    }
    cout << endl;

    for (const auto& state : states) {
      if (state) {
        cout << endl << "Recursing into state: ";
        state->print_recursive(visited);
      }
    }
  }

  cout << "Exiting print_recursive for state: " << this << endl;
}

void State::print_state_info() const {
  cout << "State Info: " << endl;
  cout << "state: " << this << endl;
  cout << "Is Accepted: " << (is_accepted ? "Yes" : "No") << endl;
  cout << "Is invalid: " << (is_invalid ? "Yes" : "No") << endl;
  cout << "Accepted Rule: " << (accepted_rule.empty() ? "None" : accepted_rule) << endl;
  cout << "Transitions: " << endl;

  for (const auto& transition : transitions) {
    cout << "  Action: " << transition.first << " -> States: ";
    for (State* state : transition.second) {
      cout << state << " ";  // This prints the memory address of the state
    }
    cout << endl;
  }
}

// StringProcessor implementations
StringProcessor::StringProcessor() {
  // ctor
}

StringProcessor::~StringProcessor() {
  // dtor
}

void StringProcessor::skip_unnecessary_spaces(int& i, string rule_definition) {
  while (i + 1 < (int)rule_definition.size() && rule_definition[i + 1] == ' ')
    i++;
}

vector<string> StringProcessor::read_rules(string address) {
  vector<string> lines;
  ifstream file(address);
  if (!file.is_open()) {
    cerr << "Error: Could not open the file: " << address << endl;
    return lines;
  }

  string line;
  while (getline(file, line)) {
    lines.push_back(line);
  }

  file.close();
  return lines;
}

string StringProcessor::remove_backslash(string org) {
  string to_remove_back_slash = "";
  if (org[0] == '\\' && org[1] == 'L')
    to_remove_back_slash = EPSILON;
  else {
    for (int i = 0; i < (int)org.size(); i++) {
      char c = org[i];
      if (!(c == '\\') || (i > 0 && org[i - 1] == '\\'))
        to_remove_back_slash += c;
    }
  }
  return to_remove_back_slash;
}

string StringProcessor::trim(const string& str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

vector<string> StringProcessor::string_processor(const string& input) {
  string trimmed_input = input;

  if ((trimmed_input.front() == '{' && trimmed_input.back() == '}') ||
      (trimmed_input.front() == '[' && trimmed_input.back() == ']')) {
    trimmed_input = trimmed_input.substr(1, trimmed_input.size() - 2);
  }
//  cout << trimmed_input << endl;
  vector<string> result;
  istringstream iss(trimmed_input);
  string token;

  while (iss >> token) {
    result.push_back(token);
  }

  return result;
}

// NonTerminal implementations
NonTerminal::NonTerminal(const std::string& name) : name(name) {}

std::string NonTerminal::getName() const {
    return name;
}

void NonTerminal::addProduction(const std::vector<Item*>& production) {
    productions.push_back(production);
}

const std::vector<std::vector<Item*>>& NonTerminal::getProductions() {
    return productions;
}

bool NonTerminal::isEqual(Item* item) const {
    return item->getName() == name;
}

// Terminal implementations
Terminal::Terminal(const std::string& name) : name(name) {}

std::string Terminal::getName() const {
    return name;
}

bool Terminal::isEqual(Item* item) const {
    return item->getName() == name;
}