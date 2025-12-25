#ifndef LL1_PARSING_TABLE_GENERATOR_H
#define LL1_PARSING_TABLE_GENERATOR_H

#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

/**
 * @class LL1ParsingTableGenerator
 * @brief Generates LL(1) parsing tables from first/follow sets and grammar rules
 *
 * This class reads first and follow sets from a text file, reads grammar rules
 * from a terminals/non-terminals file, and generates an LL(1) parsing table.
 */
class LL1ParsingTableGenerator
{
public:
    /**
     * @brief Default constructor
     */
    LL1ParsingTableGenerator();

    /**
     * @brief Generates the LL(1) parsing table
     *
     * @param firstFollowFile Path to the file containing first and follow sets
     * @param terminalsNonTerminalsFile Path to the file containing terminals and non-terminals with productions
     * @param outputFile Path where the parsing table will be saved
     */
    void generate(const string &firstFollowFile,
                  const string &terminalsNonTerminalsFile,
                  const string &outputFile);

    // getters
    const map<pair<string, string>, string> &getParsingTable() const { return parsingTable; }
    const set<string> &getTerminals() const { return terminals; }
    const set<string> &getNonTerminals() const { return nonTerminals; }
    const map<string, set<string>> &getFirstSets() const { return firstSets; }
    const map<string, set<string>> &getFollowSets() const { return followSets; }

private:
    /**
     * @struct Rule
     * @brief Represents a production rule in the grammar
     */
    struct Rule
    {
        string lhs;            ///< Left-hand side of the rule
        vector<string> rhs;    ///< Right-hand side symbols
        string originalString; ///< Formatted string representation
    };

    // Data structures
    map<string, set<string>> firstSets;             ///< First sets for non-terminals
    map<string, set<string>> followSets;            ///< Follow sets for non-terminals
    map<string, vector<Rule>> grammarRules;         ///< Grammar rules grouped by LHS
    set<string> nonTerminals;                       ///< Set of all non-terminals
    set<string> terminals;                          ///< Set of all terminals (excluding epsilon)
    map<pair<string, string>, string> parsingTable; ///< The parsing table

    // Constants
    const string LAMBDA = "\\L"; ///< Lambda/epsilon symbol representation

    /**
     * @brief Loads first and follow sets from a file
     */
    bool loadFirstFollowSets(const string &filename);

    /**
     * @brief Loads terminals, non-terminals, and grammar rules from a file
     */
    bool loadTerminalsAndNonTerminals(const string &filename);

    /**
     * @brief Parses a production string into tokens
     */
    void parseProduction(const string &production, vector<string> &tokens);

    /**
     * @brief Generates the LL(1) parsing table
     * @return true if the grammar is LL(1), false otherwise
     */
    bool generateParsingTable();

    /**
     * @brief Computes FIRST set for a sequence of symbols
     */
    set<string> computeFirstOfSequence(const vector<string> &sequence);

    /**
     * @brief Formats a production rule as a string
     */
    string formatProduction(const string &lhs, const vector<string> &rhs);

    /**
     * @brief Writes the parsing table to a file
     */
    void writeParsingTable(const string &filename);

    /**
     * @brief Trims whitespace from a string
     */
    void trim(string &str);

    /**
     * @brief Checks if a token is epsilon
     */
    bool isEpsilon(const string &token) const;

    /**
     * @brief Checks if a token is a terminal (quoted or special)
     */
    bool isTerminal(const string &token) const;

    // Make the class non-copyable
    LL1ParsingTableGenerator(const LL1ParsingTableGenerator &) = delete;
    LL1ParsingTableGenerator &operator=(const LL1ParsingTableGenerator &) = delete;
};

#endif // LL1_PARSING_TABLE_GENERATOR_H