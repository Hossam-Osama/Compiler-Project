#include "LL1ParsingTableGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

using namespace std;

LL1ParsingTableGenerator::LL1ParsingTableGenerator() {}

void LL1ParsingTableGenerator::generate(const string &firstFollowFile,
                                        const string &terminalsNonTerminalsFile,
                                        const string &outputFile)
{
    // Clear existing data
    firstSets.clear();
    followSets.clear();
    grammarRules.clear();
    nonTerminals.clear();
    terminals.clear();
    parsingTable.clear();

    // Load terminals, non-terminals and grammar rules
    if (!loadTerminalsAndNonTerminals(terminalsNonTerminalsFile))
    {
        cerr << "Error loading terminals/non-terminals from: " << terminalsNonTerminalsFile << endl;
        return;
    }

    // Load first and follow sets
    if (!loadFirstFollowSets(firstFollowFile))
    {
        cerr << "Error loading first/follow sets from: " << firstFollowFile << endl;
        return;
    }

    // Generate parsing table - if not LL(1), stop and don't write output
    if (!generateParsingTable())
    {
        cerr << "ERROR: Grammar is not LL(1). Cannot generate parsing table." << endl;
        return;
    }

    // Write parsing table to file (only if grammar is LL(1))
    writeParsingTable(outputFile);

    cout << "Parsing table successfully generated and saved to: " << outputFile << endl;
}

bool LL1ParsingTableGenerator::loadFirstFollowSets(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Cannot open first/follow file: " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Remove leading/trailing whitespace
        trim(line);

        // Check if it's a First or Follow set
        if (line.find("First(") == 0)
        {
            // Parse First set
            size_t start = 6; // Length of "First("
            size_t end = line.find(')', start);
            string symbol = line.substr(start, end - start);
            trim(symbol);

            // Find the equals sign
            size_t equalsPos = line.find('=', end);
            if (equalsPos == string::npos)
                continue;

            // Find the opening brace
            size_t bracePos = line.find('{', equalsPos);
            if (bracePos == string::npos)
                continue;

            // Find the closing brace
            size_t endBrace = line.find('}', bracePos);
            if (endBrace == string::npos)
                continue;

            // Extract symbols inside braces
            string symbols = line.substr(bracePos + 1, endBrace - bracePos - 1);

            // Split by spaces
            istringstream iss(symbols);
            string token;
            set<string> firstSet;

            while (iss >> token)
            {
                // Remove any trailing commas
                if (!token.empty() && token.back() == ',')
                {
                    token.pop_back();
                }
                if (!token.empty())
                {
                    firstSet.insert(token);
                }
            }

            firstSets[symbol] = firstSet;
        }
        else if (line.find("Follow(") == 0)
        {
            // Parse Follow set
            size_t start = 7; // Length of "Follow("
            size_t end = line.find(')', start);
            string symbol = line.substr(start, end - start);
            trim(symbol);

            // Find the equals sign
            size_t equalsPos = line.find('=', end);
            if (equalsPos == string::npos)
                continue;

            // Find the opening brace
            size_t bracePos = line.find('{', equalsPos);
            if (bracePos == string::npos)
                continue;

            // Find the closing brace
            size_t endBrace = line.find('}', bracePos);
            if (endBrace == string::npos)
                continue;

            // Extract symbols inside braces
            string symbols = line.substr(bracePos + 1, endBrace - bracePos - 1);

            // Split by spaces
            istringstream iss(symbols);
            string token;
            set<string> followSet;

            while (iss >> token)
            {
                // Remove any trailing commas
                if (!token.empty() && token.back() == ',')
                {
                    token.pop_back();
                }
                if (!token.empty())
                {
                    followSet.insert(token);
                }
            }

            followSets[symbol] = followSet;
        }
    }

    file.close();
    return true;
}

bool LL1ParsingTableGenerator::loadTerminalsAndNonTerminals(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Cannot open terminals/non-terminals file: " << filename << endl;
        return false;
    }

    string line;
    string currentNonTerminal;
    vector<Rule> currentRules;

    while (getline(file, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Remove leading/trailing whitespace
        trim(line);

        // Check if line starts with "Item: "
        if (line.find("Item: ") == 0)
        {
            // Save previous non-terminal rules if exists
            if (!currentNonTerminal.empty() && !currentRules.empty())
            {
                grammarRules[currentNonTerminal] = currentRules;
                currentRules.clear();
            }

            // Extract item name and type
            size_t itemStart = 6; // Length of "Item: "
            size_t typeStart = line.find('(', itemStart);

            if (typeStart == string::npos)
                continue;

            string itemName = line.substr(itemStart, typeStart - itemStart - 1);
            trim(itemName);

            string itemType = line.substr(typeStart + 1);
            size_t typeEnd = itemType.find(')');
            if (typeEnd != string::npos)
            {
                itemType = itemType.substr(0, typeEnd);
            }

            // Check if it's a terminal or non-terminal
            if (itemType == "Terminal")
            {
                // Add to terminals set (excluding epsilon)
                if (itemName != "'\\L'" && itemName != LAMBDA)
                {
                    terminals.insert(itemName);
                }
            }
            else if (itemType == "Non-terminal")
            {
                // Add to non-terminals set
                nonTerminals.insert(itemName);
                currentNonTerminal = itemName;
            }
        }
        // Check if line starts with "Production: "
        else if (line.find("Production: ") == 0 && !currentNonTerminal.empty())
        {
            size_t prodStart = 12; // Length of "Production: "
            string production = line.substr(prodStart);
            trim(production);

            // Parse the production
            vector<string> tokens;
            parseProduction(production, tokens);

            // Create rule
            Rule rule;
            rule.lhs = currentNonTerminal;
            rule.rhs = tokens;
            rule.originalString = formatProduction(currentNonTerminal, tokens);

            currentRules.push_back(rule);
        }
    }

    // Save the last non-terminal rules
    if (!currentNonTerminal.empty() && !currentRules.empty())
    {
        grammarRules[currentNonTerminal] = currentRules;
    }

    // Add end marker to terminals
    terminals.insert("$");

    file.close();
    return true;
}

void LL1ParsingTableGenerator::parseProduction(const string &production, vector<string> &tokens)
{
    istringstream iss(production);
    string token;

    while (iss >> token)
    {
        // Check for quoted terminals
        if (token.front() == '\'' && token.back() == '\'')
        {
            tokens.push_back(token);
        }
        else
        {
            // Check if it's epsilon
            if (token == "\\L" || token == "'\\L'")
            {
                tokens.push_back(LAMBDA);
            }
            else
            {
                // Assume it's a symbol (non-terminal or unquoted terminal)
                tokens.push_back(token);
            }
        }
    }
}

bool LL1ParsingTableGenerator::generateParsingTable()
{
    bool isLL1 = true;
    vector<string> conflictMessages;

    // Initialize parsing table with "Error" for all non-terminal/terminal pairs
    for (const auto &nonTerminal : nonTerminals)
    {
        for (const auto &terminal : terminals)
        {
            parsingTable[{nonTerminal, terminal}] = "Error";
        }
    }

    // For each production A -> α
    for (const auto &entry : grammarRules)
    {
        const string &A = entry.first;

        for (const auto &rule : entry.second)
        {
            const auto &alpha = rule.rhs;

            // Get FIRST(α)
            set<string> firstAlpha = computeFirstOfSequence(alpha);

            // Rule 1: For each terminal a in FIRST(α), add A -> α to M[A, a]
            for (const auto &a : firstAlpha)
            {
                if (isEpsilon(a))
                {
                    // Handle epsilon separately (Rule 2)
                    continue;
                }

                // Convert terminal to table format
                string terminalForTable = a;

                // Ensure terminal is quoted if needed
                if (a != "$" && (a.length() < 2 || a.front() != '\'' || a.back() != '\''))
                {
                    terminalForTable = "'" + a + "'";
                }

                // Check if this terminal is in our terminals set
                if (terminals.find(terminalForTable) == terminals.end() && terminalForTable != "$")
                {
                    // Skip if not a valid terminal
                    continue;
                }

                pair<string, string> key = {A, terminalForTable};
                if (parsingTable.find(key) != parsingTable.end())
                {
                    if (parsingTable[key] != "Error")
                    {
                        // Conflict - not LL(1)
                        string conflictMsg = "LL(1) Conflict at [" + A + ", " + terminalForTable + "]: " +
                                             parsingTable[key] + " vs " + rule.originalString;
                        conflictMessages.push_back(conflictMsg);
                        isLL1 = false;
                    }
                    else
                    {
                        parsingTable[key] = rule.originalString;
                    }
                }
            }

            // Rule 2: If ε is in FIRST(α), then for each terminal b in FOLLOW(A),
            // add A -> α to M[A, b]
            if (firstAlpha.find(LAMBDA) != firstAlpha.end())
            {
                if (followSets.find(A) != followSets.end())
                {
                    for (const auto &b : followSets[A])
                    {
                        if (isEpsilon(b))
                            continue;

                        // Convert follow symbol to table format
                        string terminalForTable = b;
                        if (b != "$" && (b.length() < 2 || b.front() != '\'' || b.back() != '\''))
                        {
                            terminalForTable = "'" + b + "'";
                        }

                        // Check if this is a valid terminal
                        if (terminals.find(terminalForTable) == terminals.end() && terminalForTable != "$")
                        {
                            // Try unquoted version
                            string unquoted = b;
                            if (b.length() >= 2 && b.front() == '\'' && b.back() == '\'')
                            {
                                unquoted = b.substr(1, b.length() - 2);
                            }
                            terminalForTable = "'" + unquoted + "'";

                            if (terminals.find(terminalForTable) == terminals.end() && unquoted != "$")
                            {
                                continue;
                            }
                        }

                        pair<string, string> key = {A, terminalForTable};
                        if (parsingTable.find(key) != parsingTable.end())
                        {
                            if (parsingTable[key] != "Error")
                            {
                                // Conflict - not LL(1)
                                string conflictMsg = "LL(1) Conflict at [" + A + ", " + terminalForTable + "]: " +
                                                     parsingTable[key] + " vs " + rule.originalString;
                                conflictMessages.push_back(conflictMsg);
                                isLL1 = false;
                            }
                            else
                            {
                                parsingTable[key] = rule.originalString;
                            }
                        }
                    }
                }
            }
        }
    }

    // Print all conflict messages if grammar is not LL(1)
    if (!isLL1)
    {
        cerr << "\n=== LL(1) CONFLICTS DETECTED ===" << endl;
        for (const auto &msg : conflictMessages)
        {
            cerr << msg << endl;
        }
        cerr << "===============================\n"
             << endl;

        // Clear the parsing table since grammar is not LL(1)
        parsingTable.clear();
    }

    return isLL1;
}

set<string> LL1ParsingTableGenerator::computeFirstOfSequence(const vector<string> &sequence)
{
    set<string> result;

    if (sequence.empty())
    {
        result.insert(LAMBDA);
        return result;
    }

    // Handle epsilon directly
    if (sequence.size() == 1 && isEpsilon(sequence[0]))
    {
        result.insert(LAMBDA);
        return result;
    }

    bool allContainEpsilon = true;

    for (const auto &symbol : sequence)
    {
        // Check if symbol is epsilon
        if (isEpsilon(symbol))
        {
            result.insert(LAMBDA);
            break;
        }

        // Check if symbol is a terminal
        if (isTerminal(symbol))
        {
            // Add the terminal to result
            if (symbol.front() == '\'' && symbol.back() == '\'')
            {
                result.insert(symbol);
            }
            else
            {
                result.insert("'" + symbol + "'");
            }
            allContainEpsilon = false;
            break;
        }

        // Symbol is non-terminal
        if (firstSets.find(symbol) != firstSets.end())
        {
            const auto &first = firstSets[symbol];

            // Add all symbols except epsilon
            for (const auto &s : first)
            {
                if (!isEpsilon(s))
                {
                    result.insert(s);
                }
            }

            // Check if epsilon is in FIRST(symbol)
            if (first.find(LAMBDA) == first.end())
            {
                allContainEpsilon = false;
                break;
            }
        }
        else
        {
            // Symbol not found in first sets
            // Assume it's a terminal we haven't seen
            result.insert(symbol);
            allContainEpsilon = false;
            break;
        }
    }

    // If all symbols in sequence can derive epsilon, add epsilon to result
    if (allContainEpsilon)
    {
        result.insert(LAMBDA);
    }

    return result;
}

string LL1ParsingTableGenerator::formatProduction(const string &lhs, const vector<string> &rhs)
{
    string result = lhs + " = ";

    if (rhs.empty() || (rhs.size() == 1 && isEpsilon(rhs[0])))
    {
        result += LAMBDA;
    }
    else
    {
        for (size_t i = 0; i < rhs.size(); ++i)
        {
            if (i > 0)
                result += " ";

            if (isEpsilon(rhs[i]))
            {
                result += LAMBDA;
            }
            else
            {
                result += rhs[i];
            }
        }
    }

    return result;
}

void LL1ParsingTableGenerator::writeParsingTable(const string &filename)
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cerr << "Cannot open output file: " << filename << endl;
        return;
    }

    // Create a sorted list of terminals for columns
    vector<string> sortedTerminals(terminals.begin(), terminals.end());

    // Sort terminals alphabetically, but keep $ at the end
    sort(sortedTerminals.begin(), sortedTerminals.end(), [](const string &a, const string &b)
         {
        if (a == "$") return false;
        if (b == "$") return true;
        return a < b; });

    // Calculate column widths
    size_t terminalWidth = 75;
    for (const auto &terminal : sortedTerminals)
    {
        terminalWidth = max(terminalWidth, terminal.length() + 2);
    }

    size_t nonTerminalWidth = 20;
    for (const auto &nonTerminal : nonTerminals)
    {
        nonTerminalWidth = max(nonTerminalWidth, nonTerminal.length() + 2);
    }

    // Write header
    string separator(terminalWidth * sortedTerminals.size() + nonTerminalWidth + 3, '=');
    file << separator << endl;
    file << setw((separator.length() - 16) / 2) << "" << "LL(1) PARSING TABLE" << endl;
    file << separator << endl;

    // Write column headers
    file << left << setw(nonTerminalWidth) << "Non-Terminal" << "|";
    for (const auto &terminal : sortedTerminals)
    {
        file << setw(terminalWidth) << terminal;
    }
    file << endl;

    // Write separator line
    file << string(nonTerminalWidth, '-') << "+";
    for (size_t i = 0; i < sortedTerminals.size(); ++i)
    {
        file << string(terminalWidth, '-');
    }
    file << endl;

    // Write table rows for each non-terminal
    vector<string> sortedNonTerminals(nonTerminals.begin(), nonTerminals.end());
    sort(sortedNonTerminals.begin(), sortedNonTerminals.end());

    for (const auto &nonTerminal : sortedNonTerminals)
    {
        file << left << setw(nonTerminalWidth) << nonTerminal << "|";

        for (const auto &terminal : sortedTerminals)
        {
            pair<string, string> key = {nonTerminal, terminal};
            string entry = "Error";

            if (parsingTable.find(key) != parsingTable.end())
            {
                entry = parsingTable[key];

                // Truncate if too long
                // if (entry.length() > terminalWidth - 2)
                // {
                //     entry = entry.substr(0, terminalWidth - 5) + "...";
                // }
            }

            file << setw(terminalWidth) << entry;
        }
        file << endl;
    }

    // Write footer
    file << separator << endl;

    file.close();
}

void LL1ParsingTableGenerator::trim(string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos)
    {
        str.clear();
        return;
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    str = str.substr(start, end - start + 1);
}

bool LL1ParsingTableGenerator::isEpsilon(const string &token) const
{
    return token == LAMBDA || token == "\\L" || token == "'\\L'";
}

bool LL1ParsingTableGenerator::isTerminal(const string &token) const
{
    // Check if it's epsilon
    if (isEpsilon(token))
        return false;

    // Check if it's quoted
    if (token.length() >= 2 && token.front() == '\'' && token.back() == '\'')
    {
        return true;
    }

    // Check if it's in terminals set
    if (terminals.find(token) != terminals.end() ||
        terminals.find("'" + token + "'") != terminals.end())
    {
        return true;
    }

    // Check if it's a special terminal
    if (token == "$")
        return true;

    // Check if it's not a non-terminal
    return nonTerminals.find(token) == nonTerminals.end();
}