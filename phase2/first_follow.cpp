#include "../headers/First_Follow.h"

#include "../headers/State.h"

#include <fstream>

using namespace std;
First_Follow::First_Follow(map<string, Item*> grammar) : grammar(grammar) {}

void First_Follow::createFirstSet() {
    for (const auto & [name, _] : grammar)
      setFirstSet(name);
}

void First_Follow::setFirstSet(const std::string& curr) {
    if (first.find(curr) != first.end())
        return;

    // Terminal case: /L or if or while
    if (isTerminal(curr)) {
        set<string> str;
        str.insert(curr);
        first[curr] = str;
        return;
    }
    auto* non_terminal = dynamic_cast<NonTerminal*>(grammar.at(curr));
    vector<vector<Item*>> productions = non_terminal->getProductions(); // ABDEF | d | eps
    for (const vector<Item*>& prod: productions) {
        // ABDEF
        int i = 0;
        for (const Item* item: prod) {
            if (item->getName() == curr)
                continue;
            if (first.find(item->getName()) == first.end())
                setFirstSet(item->getName());
            set<string> first_set_tokens_of_item = first[item->getName()];
            bool has_eps = false;
            for (const string& first_set_token: first_set_tokens_of_item) {
                if (first_set_token == LAMBDA)
                   has_eps = true;
                else {
                    first[curr].insert(first_set_token);
                }
            }
            if (has_eps)
               i++;
            else
              break;
        }
        // epsilon is in ABDEF -> Put it
        if (i == prod.size()) {
            first[curr].insert(LAMBDA);
        }
    }
}

bool First_Follow::getFollowSetOfNonTerminalUsingProduction(const string& current_production_owner_non_terminal) {
    bool changed = false;
    auto* head_non_terminal = dynamic_cast<NonTerminal*>(grammar.at(current_production_owner_non_terminal));
    // C = ABC | GHJ | BED
    for (vector<Item*> prod: head_non_terminal->getProductions()) {
        // ABC
        for (int item_to_calculate_its_follow = 0; item_to_calculate_its_follow < prod.size();
            item_to_calculate_its_follow++) {

            // A
            string item_to_calculate_its_follow_name = prod[item_to_calculate_its_follow]->getName();
            // Cannot calculate follow set of a terminal
            if (isTerminal(item_to_calculate_its_follow_name))
                continue;
            int next_item = item_to_calculate_its_follow + 1;
            for (; next_item < prod.size(); next_item++) {
                string next_item_name = prod[next_item]->getName();
                changed |=
                    addToSet(follow[item_to_calculate_its_follow_name],
                             first[next_item_name]);
                // Doesn't contain e or is a terminal -> stop iterating
                if (isTerminal(next_item_name) ||
                    (first[next_item_name].find(LAMBDA) == first[next_item_name].end()))
                    break;
            }
            if (next_item == prod.size()) { //  follow of itemToCalculateItsFollow is now owner follow
                changed |= addToSet(follow[item_to_calculate_its_follow_name],
                    follow[current_production_owner_non_terminal]);
            }
        }
    }
    return changed;
}

bool First_Follow::handleGrammarInFollowSet() {
    bool changed = false;
    for (const auto & it : grammar) {
        string curr = it.first;
        if (isTerminal(curr))
            continue;
        changed |= getFollowSetOfNonTerminalUsingProduction(curr);
    }
    return changed;
}

void First_Follow::createFollowSet(string startSymbol) {

    follow[startSymbol] = {END_SYMBOL};
    bool changed = true;
    while (changed)
        changed = handleGrammarInFollowSet();
}


bool First_Follow::addToSet(std::set<std::string>& target_set, const std::set<std::string>& source_set) {
    bool changed = false;
    for (const auto& element : source_set) {
        if (element != LAMBDA)
            changed |= target_set.insert(element).second;
    }
    return changed;
}

void First_Follow::printFirst() {
    for (int i = 0; i < 200; i++)
        cout<<'*';
    cout << endl << "FIRST_SET" << endl;
    for (const auto& [name, first_tokens] : first) {
        cout << name << " -> { ";
        for (const auto& element : first_tokens)
            cout << element << " ";
        cout << "}" << std::endl;
    }
}

void First_Follow::printFollow() {
    for (int i = 0; i < 200; i++)
        cout<<'*';
    cout << endl << "FOLLOW_SET" << endl;
    for (const auto& [name, follow_tokens] : follow) {
        cout << name << " -> { ";
        for (const auto& element : follow_tokens)
            cout << element << " ";
        cout << "}" << std::endl;
    }
}

void First_Follow::printFirstAndFollowToFile(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // First sets
    for (const auto& [name, first_tokens] : first) {
        out << "First(" << name << ") = { ";
        bool first_elem = true;

        for (const auto& elem : first_tokens) {
            if (!first_elem) out << " ";
            out << "'" << elem << "'";
            first_elem = false;
        }
        out << " }" << std::endl;
    }

    // Follow sets
    for (const auto& [name, follow_tokens] : follow) {
        out << "Follow(" << name << ") = { ";
        bool first_elem = true;

        for (const auto& elem : follow_tokens) {
            if (!first_elem) out << " ";
            out << "'" << elem << "'";
            first_elem = false;
        }
        out << " }" << std::endl;
    }

    out.close();
}


