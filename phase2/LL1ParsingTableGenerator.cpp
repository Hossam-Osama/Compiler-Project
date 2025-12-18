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
                                        const string &grammarFile,
                                        const string &outputFile)
{
    // Clear existing data
    firstSets.clear();
    followSets.clear();
    grammarRules.clear();
    nonTerminals.clear();
    terminals.clear();
    parsingTable.clear();

    // Load first and follow sets
    if (!loadFirstFollowSets(firstFollowFile))
    {
        cerr << "Error loading first/follow sets from: " << firstFollowFile << endl;
        return;
    }

    // Load grammar rules
    if (!loadGrammarRules(grammarFile))
    {
        cerr << "Error loading grammar rules from: " << grammarFile << endl;
        return;
    }

    // Build terminals set from grammar
    extractTerminalsFromGrammar();

    // Generate parsing table
    if (!generateParsingTable())
    {
        cerr << "Grammar is not LL(1)" << endl;
        return;
    }

    // Write parsing table to file
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
                    if (token == EPSILON)
                    {
                        firstSet.insert(LAMBDA); // Use LAMBDA internally
                    }
                    else
                    {
                        firstSet.insert(token);
                    }
                }
            }

            if (symbol == EPSILON)
            {
                symbol = LAMBDA;
            }

            firstSets[symbol] = firstSet;
            nonTerminals.insert(symbol);
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
                    if (token == "$")
                    {
                        followSet.insert("$");
                    }
                    else if (token == EPSILON)
                    {
                        followSet.insert(LAMBDA);
                    }
                    else
                    {
                        followSet.insert(token);
                    }
                }
            }

            followSets[symbol] = followSet;
        }
    }

    file.close();
    return true;
}

bool LL1ParsingTableGenerator::loadGrammarRules(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Cannot open grammar file: " << filename << endl;
        return false;
    }

    string line;
    string currentLHS;
    vector<vector<string>> alternatives;
    vector<string> currentRHS;

    while (getline(file, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Remove leading/trailing whitespace
        trim(line);

        // Check if line contains production symbol "::="
        size_t prodPos = line.find("::=");

        if (prodPos != string::npos)
        {
            // Save previous rule if exists
            if (!currentLHS.empty() && !alternatives.empty())
            {
                for (const auto &alt : alternatives)
                {
                    if (!alt.empty())
                    {
                        Rule rule;
                        rule.lhs = currentLHS;
                        rule.rhs = alt;
                        rule.originalString = formatProduction(currentLHS, alt);
                        grammarRules[currentLHS].push_back(rule);
                    }
                }
                alternatives.clear();
            }

            // Start new rule
            currentLHS = line.substr(0, prodPos);
            trim(currentLHS);
            nonTerminals.insert(currentLHS);

            // Parse RHS
            string rhs = line.substr(prodPos + 3);
            trim(rhs);

            // Split by alternatives
            vector<string> altStrings;
            splitAlternatives(rhs, altStrings);

            // Parse each alternative
            for (const auto &altStr : altStrings)
            {
                currentRHS.clear();
                parseTokens(altStr, currentRHS);
                alternatives.push_back(currentRHS);
            }
        }
        else if (!currentLHS.empty())
        {
            // Continuation of previous rule
            trim(line);

            // Split by alternatives if any
            vector<string> altStrings;
            splitAlternatives(line, altStrings);

            // Parse each alternative
            for (const auto &altStr : altStrings)
            {
                currentRHS.clear();
                parseTokens(altStr, currentRHS);
                alternatives.push_back(currentRHS);
            }
        }
    }

    // Save the last rule
    if (!currentLHS.empty() && !alternatives.empty())
    {
        for (const auto &alt : alternatives)
        {
            if (!alt.empty())
            {
                Rule rule;
                rule.lhs = currentLHS;
                rule.rhs = alt;
                rule.originalString = formatProduction(currentLHS, alt);
                grammarRules[currentLHS].push_back(rule);
            }
        }
    }

    file.close();
    return true;
}

void LL1ParsingTableGenerator::splitAlternatives(const string &str, vector<string> &alternatives)
{
    string current;
    bool inQuotes = false;
    bool escape = false;

    for (size_t i = 0; i < str.length(); ++i)
    {
        char c = str[i];

        if (escape)
        {
            current += c;
            escape = false;
            continue;
        }

        if (c == '\\')
        {
            escape = true;
            current += c;
            continue;
        }

        if (c == '\'')
        {
            inQuotes = !inQuotes;
            current += c;
        }
        else if (c == '|' && !inQuotes)
        {
            // Found alternative separator
            trim(current);
            if (!current.empty())
            {
                alternatives.push_back(current);
            }
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    // Add the last alternative
    trim(current);
    if (!current.empty())
    {
        alternatives.push_back(current);
    }
}

void LL1ParsingTableGenerator::parseTokens(const string &str, vector<string> &tokens)
{
    string token;
    bool inQuotes = false;
    bool escape = false;

    for (size_t i = 0; i < str.length(); ++i)
    {
        char c = str[i];

        if (escape)
        {
            token += c;
            escape = false;
            continue;
        }

        if (c == '\\')
        {
            escape = true;
            continue;
        }

        if (c == '\'')
        {
            if (inQuotes)
            {
                // End of quoted terminal
                token += c;
                if (!token.empty())
                {
                    tokens.push_back(token);
                }
                token.clear();
            }
            else
            {
                // Start of quoted terminal
                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
                token += c;
            }
            inQuotes = !inQuotes;
        }
        else if (c == ' ' && !inQuotes)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }

    if (!token.empty())
    {
        tokens.push_back(token);
    }
}

void LL1ParsingTableGenerator::extractTerminalsFromGrammar()
{
    // Add standard terminals
    terminals.insert("$");

    // Extract terminals from grammar rules
    for (const auto &entry : grammarRules)
    {
        for (const auto &rule : entry.second)
        {
            for (const auto &token : rule.rhs)
            {
                // Check if token is terminal (enclosed in single quotes)
                if (token.length() >= 2 && token.front() == '\'' && token.back() == '\'')
                {
                    terminals.insert(token);
                }
                // Check if it's epsilon (should be handled specially)
                else if (token == LAMBDA || token == EPSILON)
                {
                    // Don't add epsilon to terminals - it's a special symbol
                    continue;
                }
                // Otherwise, it could be a non-terminal
                else if (terminals.find(token) == terminals.end() &&
                         nonTerminals.find(token) == nonTerminals.end())
                {
                    // If it's not in nonTerminals and looks like uppercase, might be token
                    bool allUpper = true;
                    for (char c : token)
                    {
                        if (!isupper(c) && c != '_')
                        {
                            allUpper = false;
                            break;
                        }
                    }
                    if (allUpper)
                    {
                        nonTerminals.insert(token);
                    }
                }
            }
        }
    }

    // Also add terminals from first sets
    for (const auto &entry : firstSets)
    {
        for (const auto &symbol : entry.second)
        {
            if (symbol != LAMBDA && symbol != EPSILON &&
                nonTerminals.find(symbol) == nonTerminals.end())
            {
                terminals.insert(symbol);
            }
        }
    }

    // Also add terminals from follow sets
    for (const auto &entry : followSets)
    {
        for (const auto &symbol : entry.second)
        {
            if (symbol != LAMBDA && symbol != EPSILON &&
                symbol != "$" &&
                nonTerminals.find(symbol) == nonTerminals.end())
            {
                terminals.insert(symbol);
            }
        }
    }
}

bool LL1ParsingTableGenerator::generateParsingTable()
{
    bool isLL1 = true;

    // Initialize parsing table with "Error"
    for (const auto &nonTerminal : nonTerminals)
    {
        for (const auto &terminal : terminals)
        {
            parsingTable[{nonTerminal, terminal}] = "Error";
        }
        // Also add for $
        parsingTable[{nonTerminal, "$"}] = "Error";
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
                if (a == LAMBDA)
                {
                    // Handle epsilon separately (Rule 2)
                    continue;
                }

                pair<string, string> key = {A, a};
                if (parsingTable.find(key) != parsingTable.end())
                {
                    if (parsingTable[key] != "Error")
                    {
                        // Conflict - not LL(1)
                        cerr << "LL(1) Conflict at [" << A << ", " << a << "]: "
                                  << parsingTable[key] << " vs " << rule.originalString << endl;
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
                        if (b == LAMBDA)
                            continue; // Skip epsilon from follow sets

                        pair<string, string> key = {A, b};
                        if (parsingTable.find(key) != parsingTable.end())
                        {
                            if (parsingTable[key] != "Error")
                            {
                                // Conflict - not LL(1)
                                cerr << "LL(1) Conflict at [" << A << ", " << b << "]: "
                                          << parsingTable[key] << " vs " << rule.originalString << endl;
                                isLL1 = false;
                            }
                            else
                            {
                                parsingTable[key] = rule.originalString;
                            }
                        }
                    }
                }

                // Also handle $ specifically
                pair<string, string> key = {A, "$"};
                if (parsingTable.find(key) != parsingTable.end() && parsingTable[key] == "Error")
                {
                    // Check if $ is in FOLLOW(A)
                    bool dollarInFollow = false;
                    if (followSets.find(A) != followSets.end())
                    {
                        dollarInFollow = (followSets[A].find("$") != followSets[A].end());
                    }

                    // For ε productions, we also need to consider that they might be valid
                    // when the input is empty (i.e., at the end of string, represented by $)
                    if (dollarInFollow || (alpha.size() == 1 && (alpha[0] == LAMBDA || alpha[0] == EPSILON)))
                    {
                        parsingTable[key] = rule.originalString;
                    }
                }
            }
        }
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
    if (sequence.size() == 1 && (sequence[0] == LAMBDA || sequence[0] == EPSILON))
    {
        result.insert(LAMBDA);
        return result;
    }

    bool allContainEpsilon = true;

    for (const auto &symbol : sequence)
    {
        // Check if symbol is epsilon
        if (symbol == LAMBDA || symbol == EPSILON)
        {
            result.insert(LAMBDA);
            break;
        }

        // Check if symbol is terminal (enclosed in quotes)
        if (symbol.length() >= 2 && symbol.front() == '\'' && symbol.back() == '\'')
        {
            result.insert(symbol);
            allContainEpsilon = false;
            break;
        }

        // Check if symbol is a regular terminal (not in nonTerminals)
        if (terminals.find(symbol) != terminals.end() &&
            nonTerminals.find(symbol) == nonTerminals.end())
        {
            result.insert(symbol);
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
                if (s != LAMBDA)
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
            // Symbol not found in first sets - treat as terminal
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
    string result = lhs + " ::= ";

    if (rhs.empty() || (rhs.size() == 1 && (rhs[0] == LAMBDA || rhs[0] == EPSILON)))
    {
        result += EPSILON; // Use ε for display
    }
    else
    {
        for (size_t i = 0; i < rhs.size(); ++i)
        {
            if (i > 0)
                result += " ";

            // If it's lambda internally, display as epsilon
            if (rhs[i] == LAMBDA)
            {
                result += EPSILON;
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
    vector<string> sortedTerminals;
    for (const auto &terminal : terminals)
    {
        sortedTerminals.push_back(terminal);
    }
    // Add $ at the end
    sortedTerminals.push_back("$");

    // Sort terminals alphabetically, but keep $ at the end
    sort(sortedTerminals.begin(), sortedTerminals.end() - 1);

    // Calculate column width for terminal headers
    size_t terminalWidth = 50;
    for (const auto &terminal : sortedTerminals)
    {
        terminalWidth = max(terminalWidth, terminal.length() + 2);
    }

    // Calculate non-terminal column width
    size_t nonTerminalWidth = 25;
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
                if (entry.length() > terminalWidth - 2)
                {
                    entry = entry.substr(0, terminalWidth - 5) + "...";
                }
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