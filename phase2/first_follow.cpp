#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

// ----------------- Helpers -----------------

string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

// Normalize non-terminal by removing trailing "::"
string normalizeNonTerminal(const string &s)
{
    string t = trim(s);
    if (t.size() >= 2 && t.substr(t.size() - 2) == "::")
        t = t.substr(0, t.size() - 2);
    return t;
}

// ----------------- Headers -----------------

vector<string> split(const string &str, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream stream(str);
    while (getline(stream, token, delimiter))
    {
        tokens.push_back(trim(token));
    }
    return tokens;
}

vector<string> tokenize(const string &production)
{
    vector<string> tokens;
    string token;
    istringstream stream(production);
    while (stream >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

// ----------------- FIRST -----------------

void calculateFirst(const map<string, vector<vector<string>>> &grammar, map<string, set<string>> &first)
{
    set<string> terminals;
    for (const auto &rule : grammar)
    {
        for (const auto &prod : rule.second)
        {
            for (const auto &sym : prod)
            {
                if (grammar.find(sym) == grammar.end() && sym != "\\L")
                    terminals.insert(sym);
            }
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (const auto &rule : grammar)
        {
            string nt = normalizeNonTerminal(rule.first);
            for (const auto &prod : rule.second)
            {
                bool epsilonInAll = true;

                for (const string &symbol : prod)
                {
                    if (terminals.count(symbol) > 0)
                    {
                        if (first[nt].insert(symbol).second)
                            changed = true;
                        epsilonInAll = false;
                        break;
                    }
                    else
                    {
                        string key = normalizeNonTerminal(symbol);
                        size_t prevSize = first[nt].size();
                        first[nt].insert(first[key].begin(), first[key].end());
                        first[nt].erase("\\L");
                        if (first[key].count("\\L") == 0)
                        {
                            epsilonInAll = false;
                            break;
                        }
                        if (first[nt].size() != prevSize)
                            changed = true;
                    }
                }

                if (epsilonInAll)
                {
                    if (first[nt].insert("\\L").second)
                        changed = true;
                }
            }
        }
    }
}

// ----------------- FOLLOW -----------------

void calculateFollow(const map<string, vector<vector<string>>> &grammar,
                     const map<string, set<string>> &first,
                     map<string, set<string>> &follow,
                     const string &startSymbol)
{
    set<string> terminals;
    for (const auto &rule : grammar)
    {
        for (const auto &prod : rule.second)
        {
            for (const auto &sym : prod)
            {
                if (grammar.find(sym) == grammar.end() && sym != "\\L")
                    terminals.insert(sym);
            }
        }
    }

    string start = normalizeNonTerminal(startSymbol);
    follow[start].insert("$");
    bool changed = true;

    while (changed)
    {
        changed = false;

        for (const auto &rule : grammar)
        {
            string lhs = normalizeNonTerminal(rule.first);

            for (const auto &prod : rule.second)
            {
                for (size_t i = 0; i < prod.size(); ++i)
                {
                    string symbol = prod[i];
                    if (terminals.count(symbol) > 0 || symbol == "\\L")
                        continue;

                    string symKey = normalizeNonTerminal(symbol);
                    set<string> trailer;
                    bool epsilonInAll = true;

                    for (size_t j = i + 1; j < prod.size(); ++j)
                    {
                        string nextSym = prod[j];
                        if (terminals.count(nextSym) > 0)
                        {
                            trailer.insert(nextSym);
                            epsilonInAll = false;
                            break;
                        }
                        else
                        {
                            string nextKey = normalizeNonTerminal(nextSym);
                            for (const string &f : first.at(nextKey))
                            {
                                if (f != "\\L")
                                    trailer.insert(f);
                            }
                            if (first.at(nextKey).count("\\L") == 0)
                            {
                                epsilonInAll = false;
                                break;
                            }
                        }
                    }

                    if (i + 1 == prod.size() || epsilonInAll)
                        trailer.insert(follow[lhs].begin(), follow[lhs].end());

                    size_t prevSize = follow[symKey].size();
                    follow[symKey].insert(trailer.begin(), trailer.end());
                    if (follow[symKey].size() != prevSize)
                        changed = true;
                }
            }
        }
    }
}
