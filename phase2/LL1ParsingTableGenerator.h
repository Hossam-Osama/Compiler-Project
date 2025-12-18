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
 * from another text file, and generates an LL(1) parsing table. It validates
 * whether the grammar is LL(1) and produces a formatted output table.
 *
 * The class properly handles grammar rules with alternatives (| symbol),
 * splitting them into separate productions for accurate parsing table generation.
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
     * @param grammarFile Path to the file containing grammar rules
     * @param outputFile Path where the parsing table will be saved
     *
     * The method performs the following steps:
     * 1. Loads first and follow sets from firstFollowFile
     * 2. Loads grammar rules from grammarFile, splitting alternatives into separate productions
     * 3. Extracts terminals and non-terminals
     * 4. Generates the parsing table
     * 5. Writes the table to outputFile
     *
     * If the grammar is not LL(1), an error message is printed to stderr.
     */
    void generate(const string &firstFollowFile,
                  const string &grammarFile,
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
     *
     * For rules with alternatives like "A ::= B | C", this creates
     * separate Rule objects: one for "A ::= B" and another for "A ::= C"
     */
    struct Rule
    {
        string lhs;              ///< Left-hand side of the rule
        vector<string> rhs; ///< Right-hand side symbols
        string originalString;   ///< Formatted string representation of this specific production
    };

    // Data structures
    map<string, set<string>> firstSets;                  ///< First sets for non-terminals
    map<string, set<string>> followSets;                 ///< Follow sets for non-terminals
    map<string, vector<Rule>> grammarRules;                   ///< Grammar rules grouped by LHS (each alternative stored separately)
    set<string> nonTerminals;                                      ///< Set of all non-terminals
    set<string> terminals;                                         ///< Set of all terminals
    map<pair<string, string>, string> parsingTable; ///< The parsing table [non-terminal][terminal] -> production

    // Constants
    const string LAMBDA = "\\L"; ///< Lambda symbol representation
    const string EPSILON = "ε";  ///< Epsilon symbol representation

    const map<string, vector<Rule>> &getGrammarRules() const { return grammarRules; }

    /**
     * @brief Loads first and follow sets from a file
     *
     * @param filename Path to the file containing first and follow sets
     * @return true if loading was successful, false otherwise
     *
     * Expected format:
     * First(SYMBOL) = { token1 token2 ... }
     * Follow(SYMBOL) = { token1 token2 ... }
     */
    bool loadFirstFollowSets(const string &filename);

    /**
     * @brief Loads grammar rules from a file
     *
     * @param filename Path to the file containing grammar rules
     * @return true if loading was successful, false otherwise
     *
     * Expected format:
     * LHS ::= RHS1 | RHS2 | ...
     * Supports multi-line rules and escape sequences.
     * Each alternative is stored as a separate Rule object.
     */
    bool loadGrammarRules(const string &filename);

    /**
     * @brief Splits a string into alternatives separated by | while respecting quoted terminals
     *
     * @param str The string to split
     * @param alternatives Output vector to store the alternatives
     *
     * Example: "A | B | C" -> ["A", "B", "C"]
     * The method properly handles quoted terminals so that '|' inside quotes is not treated as a separator.
     */
    void splitAlternatives(const string &str, vector<string> &alternatives);

    /**
     * @brief Parses a string into tokens for grammar rules
     *
     * @param str The string to parse
     * @param tokens Output vector to store parsed tokens
     *
     * Handles quoted terminals, escape sequences, and spaces.
     * Example: "'int' IDENTIFIER" -> ["'int'", "IDENTIFIER"]
     */
    void parseTokens(const string &str, vector<string> &tokens);

    /**
     * @brief Extracts terminal symbols from the grammar rules
     *
     * Populates the terminals set by examining all grammar rules
     * and first/follow sets. Terminals are identified as symbols
     * enclosed in single quotes.
     */
    void extractTerminalsFromGrammar();

    /**
     * @brief Generates the LL(1) parsing table
     *
     * @return true if the grammar is LL(1), false otherwise
     *
     * Implements the standard LL(1) parsing table construction algorithm:
     * 1. For each production A -> α
     * 2. For each terminal a in FIRST(α), add A -> α to M[A, a]
     * 3. If ε ∈ FIRST(α), add A -> α to M[A, b] for each b in FOLLOW(A)
     *
     * Reports conflicts if the grammar is not LL(1).
     */
    bool generateParsingTable();

    /**
     * @brief Computes FIRST set for a sequence of symbols
     *
     * @param sequence The sequence of symbols
     * @return Set of terminals that can begin strings derived from the sequence
     */
    set<string> computeFirstOfSequence(const vector<string> &sequence);

    /**
     * @brief Creates a formatted string for a production rule
     *
     * @param lhs The left-hand side non-terminal
     * @param rhs The right-hand side symbols
     * @return Formatted string "LHS ::= RHS"
     */
    string formatProduction(const string &lhs, const vector<string> &rhs);

    /**
     * @brief Writes the parsing table to a file
     *
     * @param filename Path to the output file
     *
     * Generates a formatted table with non-terminals as rows,
     * terminals as columns, and production rules as entries.
     * The table is formatted to match the specified output format.
     */
    void writeParsingTable(const string &filename);

    /**
     * @brief Trims whitespace from a string
     *
     * @param str The string to trim (modified in place)
     */
    void trim(string &str);

    // Make the class non-copyable
    LL1ParsingTableGenerator(const LL1ParsingTableGenerator &) = delete;
    LL1ParsingTableGenerator &operator=(const LL1ParsingTableGenerator &) = delete;
};

#endif // LL1_PARSING_TABLE_GENERATOR_H