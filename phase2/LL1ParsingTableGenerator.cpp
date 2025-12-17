#include "LL1ParsingTableGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

LL1ParsingTableGenerator::LL1ParsingTableGenerator() {}

void LL1ParsingTableGenerator::generate(const std::string &firstFollowFile,
                                        const std::string &grammarFile,
                                        const std::string &outputFile)
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
        std::cerr << "Error loading first/follow sets from: " << firstFollowFile << std::endl;
        return;
    }

    // Load grammar rules
    if (!loadGrammarRules(grammarFile))
    {
        std::cerr << "Error loading grammar rules from: " << grammarFile << std::endl;
        return;
    }

    // Build terminals set from grammar
    extractTerminalsFromGrammar();

    // Generate parsing table
    if (!generateParsingTable())
    {
        std::cerr << "Grammar is not LL(1)" << std::endl;
        return;
    }

    // Write parsing table to file
    writeParsingTable(outputFile);

    std::cout << "Parsing table successfully generated and saved to: " << outputFile << std::endl;
}

bool LL1ParsingTableGenerator::loadFirstFollowSets(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Cannot open first/follow file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
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
            std::string symbol = line.substr(start, end - start);
            trim(symbol);

            // Find the equals sign
            size_t equalsPos = line.find('=', end);
            if (equalsPos == std::string::npos)
                continue;

            // Find the opening brace
            size_t bracePos = line.find('{', equalsPos);
            if (bracePos == std::string::npos)
                continue;

            // Find the closing brace
            size_t endBrace = line.find('}', bracePos);
            if (endBrace == std::string::npos)
                continue;

            // Extract symbols inside braces
            std::string symbols = line.substr(bracePos + 1, endBrace - bracePos - 1);

            // Split by spaces
            std::istringstream iss(symbols);
            std::string token;
            std::set<std::string> firstSet;

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
            std::string symbol = line.substr(start, end - start);
            trim(symbol);

            // Find the equals sign
            size_t equalsPos = line.find('=', end);
            if (equalsPos == std::string::npos)
                continue;

            // Find the opening brace
            size_t bracePos = line.find('{', equalsPos);
            if (bracePos == std::string::npos)
                continue;

            // Find the closing brace
            size_t endBrace = line.find('}', bracePos);
            if (endBrace == std::string::npos)
                continue;

            // Extract symbols inside braces
            std::string symbols = line.substr(bracePos + 1, endBrace - bracePos - 1);

            // Split by spaces
            std::istringstream iss(symbols);
            std::string token;
            std::set<std::string> followSet;

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

bool LL1ParsingTableGenerator::loadGrammarRules(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Cannot open grammar file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::string currentLHS;
    std::vector<std::vector<std::string>> alternatives;
    std::vector<std::string> currentRHS;

    while (std::getline(file, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Remove leading/trailing whitespace
        trim(line);

        // Check if line contains production symbol "::="
        size_t prodPos = line.find("::=");

        if (prodPos != std::string::npos)
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
            std::string rhs = line.substr(prodPos + 3);
            trim(rhs);

            // Split by alternatives
            std::vector<std::string> altStrings;
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
            std::vector<std::string> altStrings;
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

void LL1ParsingTableGenerator::splitAlternatives(const std::string &str, std::vector<std::string> &alternatives)
{
    std::string current;
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

void LL1ParsingTableGenerator::parseTokens(const std::string &str, std::vector<std::string> &tokens)
{
    std::string token;
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
                        if (!std::isupper(c) && c != '_')
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
        const std::string &A = entry.first;

        for (const auto &rule : entry.second)
        {
            const auto &alpha = rule.rhs;

            // Get FIRST(α)
            std::set<std::string> firstAlpha = computeFirstOfSequence(alpha);

            // Rule 1: For each terminal a in FIRST(α), add A -> α to M[A, a]
            for (const auto &a : firstAlpha)
            {
                if (a == LAMBDA)
                {
                    // Handle epsilon separately (Rule 2)
                    continue;
                }

                std::pair<std::string, std::string> key = {A, a};
                if (parsingTable.find(key) != parsingTable.end())
                {
                    if (parsingTable[key] != "Error")
                    {
                        // Conflict - not LL(1)
                        std::cerr << "LL(1) Conflict at [" << A << ", " << a << "]: "
                                  << parsingTable[key] << " vs " << rule.originalString << std::endl;
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

                        std::pair<std::string, std::string> key = {A, b};
                        if (parsingTable.find(key) != parsingTable.end())
                        {
                            if (parsingTable[key] != "Error")
                            {
                                // Conflict - not LL(1)
                                std::cerr << "LL(1) Conflict at [" << A << ", " << b << "]: "
                                          << parsingTable[key] << " vs " << rule.originalString << std::endl;
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
                std::pair<std::string, std::string> key = {A, "$"};
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

std::set<std::string> LL1ParsingTableGenerator::computeFirstOfSequence(const std::vector<std::string> &sequence)
{
    std::set<std::string> result;

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

std::string LL1ParsingTableGenerator::formatProduction(const std::string &lhs, const std::vector<std::string> &rhs)
{
    std::string result = lhs + " ::= ";

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

void LL1ParsingTableGenerator::writeParsingTable(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Cannot open output file: " << filename << std::endl;
        return;
    }

    // Create a sorted list of terminals for columns
    std::vector<std::string> sortedTerminals;
    for (const auto &terminal : terminals)
    {
        sortedTerminals.push_back(terminal);
    }
    // Add $ at the end
    sortedTerminals.push_back("$");

    // Sort terminals alphabetically, but keep $ at the end
    std::sort(sortedTerminals.begin(), sortedTerminals.end() - 1);

    // Calculate column width for terminal headers
    size_t terminalWidth = 50;
    for (const auto &terminal : sortedTerminals)
    {
        terminalWidth = std::max(terminalWidth, terminal.length() + 2);
    }

    // Calculate non-terminal column width
    size_t nonTerminalWidth = 25;
    for (const auto &nonTerminal : nonTerminals)
    {
        nonTerminalWidth = std::max(nonTerminalWidth, nonTerminal.length() + 2);
    }

    // Write header
    std::string separator(terminalWidth * sortedTerminals.size() + nonTerminalWidth + 3, '=');
    file << separator << std::endl;
    file << std::setw((separator.length() - 16) / 2) << "" << "LL(1) PARSING TABLE" << std::endl;
    file << separator << std::endl;

    // Write column headers
    file << std::left << std::setw(nonTerminalWidth) << "Non-Terminal" << "|";
    for (const auto &terminal : sortedTerminals)
    {
        file << std::setw(terminalWidth) << terminal;
    }
    file << std::endl;

    // Write separator line
    file << std::string(nonTerminalWidth, '-') << "+";
    for (size_t i = 0; i < sortedTerminals.size(); ++i)
    {
        file << std::string(terminalWidth, '-');
    }
    file << std::endl;

    // Write table rows for each non-terminal
    std::vector<std::string> sortedNonTerminals(nonTerminals.begin(), nonTerminals.end());
    std::sort(sortedNonTerminals.begin(), sortedNonTerminals.end());

    for (const auto &nonTerminal : sortedNonTerminals)
    {
        file << std::left << std::setw(nonTerminalWidth) << nonTerminal << "|";

        for (const auto &terminal : sortedTerminals)
        {
            std::pair<std::string, std::string> key = {nonTerminal, terminal};
            std::string entry = "Error";

            if (parsingTable.find(key) != parsingTable.end())
            {
                entry = parsingTable[key];

                // Truncate if too long
                if (entry.length() > terminalWidth - 2)
                {
                    entry = entry.substr(0, terminalWidth - 5) + "...";
                }
            }

            file << std::setw(terminalWidth) << entry;
        }
        file << std::endl;
    }

    // Write footer
    file << separator << std::endl;

    file.close();
}

void LL1ParsingTableGenerator::trim(std::string &str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        str.clear();
        return;
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    str = str.substr(start, end - start + 1);
}