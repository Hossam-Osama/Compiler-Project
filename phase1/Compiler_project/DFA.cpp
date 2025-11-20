#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <stack>
#include <sstream>
#include <unordered_set>
#include "RegexToFSA.h"


using namespace std;
typedef set<string> State;



// DFA Structure
struct DFA {
    set<set<string>> states;                                // DFA states as sets of NFA states
    map<set<string>, map<string, set<string>>> transitions; // DFA transitions
    set<string> starting;                                   // Starting state
    map<set<string>, unordered_set<string>> accepting;      // Accepting states with associated tokens
};
//-------------------------------------part2-----------------------
// Function to split a string by a delimiter
pair<string, string> split_accepting(const string& str) {
    stringstream ss(str);
    string state, token;
    ss >> state >> token;
    return {state, token};
}

// Compute epsilon closure of a set of states
set<string> epsilon_closure(const NFS& nfa, const set<string>& states) {
    set<string> closure = states;
    stack<string> stack;

    for (const auto& state : states) {
        stack.push(state);
    }

    while (!stack.empty()) {
        string state = stack.top();
        stack.pop();

        if (nfa.trans.count(state) && nfa.trans.at(state).count(" ")) { // Epsilon transitions
            for (const auto& target : nfa.trans.at(state).at(" ")) {
                if (closure.insert(target).second) {
                    stack.push(target);
                }
            }
        }
    }
    return closure;
}

// Convert NFA to DFA and return the DFA
DFA nfa_to_dfa(const NFS& nfa) {
    DFA dfa;
    set<set<string>> unprocessed_states;

    set<string> start_closure = epsilon_closure(nfa, {nfa.starting});
    unprocessed_states.insert(start_closure);
    dfa.states.insert(start_closure);
    dfa.starting = start_closure;

    while (!unprocessed_states.empty()) {
        set<string> current_state = *unprocessed_states.begin();
        unprocessed_states.erase(unprocessed_states.begin());

        map<string, set<string>> transitions;

        // Compute transitions for each symbol
        for (const auto& state : current_state) {
            if (nfa.trans.count(state)) {
                for (const auto& [symbol, targets] : nfa.trans.at(state)) {
                    if (symbol != " ") { // Skip epsilon transitions
                        transitions[symbol].insert(targets.begin(), targets.end());
                    }
                }
            }
        }

        // Process transitions and add new states
        for (auto& [symbol, target_states] : transitions) {
            set<string> closure = epsilon_closure(nfa, target_states);
            if (dfa.states.insert(closure).second) {
                unprocessed_states.insert(closure);
            }
            dfa.transitions[current_state][symbol] = closure;
        }

        // Check if the current DFA state is accepting and collect tokens
        unordered_set<string> tokens;
        for (const auto& state : current_state) {
            for (const auto& accepting : nfa.accepting) {
                auto [accept_state, token] = split_accepting(accepting);
                if (state == accept_state) {
                    tokens.insert(token);
                }
            }
        }
        if (!tokens.empty()) {
            dfa.accepting[current_state] = tokens;
        }
    }

    return dfa;
}

// Helper function to print DFA
void print_dfa(const DFA& dfa) {
    cout << "DFA States and Transitions:\n";
    for (const auto& [state, transitions] : dfa.transitions) {
        cout << "{ ";
        for (const auto& s : state) cout << s << " ";
        cout << "} -> ";
        for (const auto& [symbol, target_state] : transitions) {
            cout << symbol << " -> { ";
            for (const auto& t : target_state) cout << t << " ";
            cout << "} ";
        }
        cout << "\n";
    }

    cout << "DFA Accepting States:\n";
    for (const auto& [state, tokens] : dfa.accepting) {
        cout << "{ ";
        for (const auto& s : state) cout << s << " ";
        cout << "} with tokens: ";
        for (const auto& token : tokens) cout << token << " ";
        cout << "\n";
    }
}

//-------------------------------------------part 3 minimization-------------
bool CompareStates(State a, State b){
  if(a.size() != b.size())return false;
  auto it1 = a.begin();
  auto it2 = b.begin();
  while(it1 != a.end()){
    if(*it1 != *it2)return false;
    it1++;
    it2++;
  }
  return true;
}

// Minimize DFA using Moore's algorithm
DFA minimizeDFA(const DFA& dfa) {
    DFA minDFA;

    // Map each DFA state to an index
    vector<State> stateList(dfa.states.begin(), dfa.states.end());
    map<State, int> idOf;
    for (int i = 0; i < stateList.size(); i++)
        idOf[stateList[i]] = i;

    int n = stateList.size();

    // Determine alphabet
    set<string> alphabet;
    for (auto& [s, trans] : dfa.transitions)
        for (auto& [sym, ns] : trans)
            alphabet.insert(sym);

    // Build transition table by IDs
    vector<map<string,int>> T(n);
    for (int i = 0; i < n; i++) {
        const State& st = stateList[i];
        if (dfa.transitions.count(st)) {
            for (auto& [sym, ns] : dfa.transitions.at(st))
                T[i][sym] = idOf[ns];
        }
    }

    // Initial partition
    vector<int> part(n, 0);
    vector<vector<int>> P;

    vector<int> nonacc;

    // FIXED: use vector<string> as map key (sorted)
    map<vector<string>, int> accGroups;

    for (int i = 0; i < n; i++) {
        const State& s = stateList[i];

        if (!dfa.accepting.count(s)) {
            nonacc.push_back(i);
            continue;
        }

        // convert unordered_set<string> -> sorted vector<string>
        vector<string> tokVec(
            dfa.accepting.at(s).begin(),
            dfa.accepting.at(s).end()
        );
        sort(tokVec.begin(), tokVec.end());

        if (!accGroups.count(tokVec)) {
            int newID = accGroups.size() + 1;
            accGroups[tokVec] = newID;
        }
        part[i] = accGroups[tokVec];
    }

    if (!nonacc.empty())
        P.push_back(nonacc);

    for (auto& [tokSet, pid] : accGroups) {
        vector<int> group;
        for (int i = 0; i < n; i++)
            if (part[i] == pid)
                group.push_back(i);
        P.push_back(group);
    }

    // Hopcroft refinement
    bool changed = true;
    while (changed) {
        changed = false;
        vector<vector<int>> newP;

        for (auto& group : P) {
            map<vector<int>, vector<int>> blocks;

            for (int s : group) {
                vector<int> sig;
                for (auto& sym : alphabet) {
                    if (T[s].count(sym))
                        sig.push_back(part[T[s][sym]]);
                    else
                        sig.push_back(-1);
                }
                blocks[sig].push_back(s);
            }

            if (blocks.size() == 1)
                newP.push_back(group);
            else {
                changed = true;
                for (auto& [sig, b] : blocks)
                    newP.push_back(b);
            }
        }

        part.assign(n, 0);
        for (int i = 0; i < newP.size(); i++)
            for (int s : newP[i])
                part[s] = i;

        P = newP;
    }

    // Build minimized DFA
    vector<State> newStates(P.size());
    for (int i = 0; i < P.size(); i++)
        for (int s : P[i])
            newStates[i].insert(
                stateList[s].begin(),
                stateList[s].end()
            );

    for (auto& s : newStates)
        minDFA.states.insert(s);

    // Starting state
    int startID = part[idOf[dfa.starting]];
    minDFA.starting = newStates[startID];

    // Transitions
    for (int i = 0; i < P.size(); i++) {
        map<string, State> row;
        int rep = P[i][0];

        for (auto& sym : alphabet) {
            if (T[rep].count(sym)) {
                int to = part[T[rep][sym]];
                row[sym] = newStates[to];
            }
        }
        minDFA.transitions[newStates[i]] = row;
    }

    // Accepting states
    for (int i = 0; i < P.size(); i++) {

        unordered_set<string> tokenSet;

        for (int s : P[i]) {
            const State& orig = stateList[s];

            if (dfa.accepting.count(orig)) {
                for (auto& tok : dfa.accepting.at(orig))
                    tokenSet.insert(tok);
            }
        }

        if (!tokenSet.empty())
            minDFA.accepting[newStates[i]] = tokenSet;
    }

    return minDFA;
}

string setToString(const set<string>& s) {
    string result = "{";
    for (auto it = s.begin(); it != s.end(); ++it) {
        result += *it;
        if (next(it) != s.end()) result += ", ";
    }
    result += "}";
    return result;
}

// Function to write DFA transitions to a file
void writeDFATransitionsToFile(const DFA& dfa, const string& filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cerr << "Error: Unable to open file " << filename << endl;
        return;
    }

    outFile << "DFA Transitions:\n";
    for (const auto& [state, transitions] : dfa.transitions) {
        outFile << "State: " << setToString(state) << "\n";
        for (const auto& [input, nextState] : transitions) {
            outFile << "  On input '" << input << "' -> " << setToString(nextState) << "\n";
        }
        outFile << "\n";
    }

    outFile.close();
    cout << "DFA transitions written to " << filename << endl;
}

//---------------------------------------part 4 consume the dfa-----------------

vector<string> splitBywhitespace(const string& line) {
    vector<string> words;
    string word;
    stringstream ss(line);

    while (ss >> word) {
        words.push_back(word);
    }

    return words;
}



vector<string> tokenize(const DFA& dfa, const string& input, const vector<string>& priority) {
    vector<string> tokens;        // Store resulting tokens
    size_t pos = 0;               // Current position in the input string
    const set<string>* currState; // Pointer to the current DFA state

    while (pos < input.length()) {
        currState = &dfa.starting; // Reset to starting state for each token
        size_t lastMatchPos = pos;
        const set<string>* lastAcceptingState = nullptr;
        vector<string> possibleTokens; // Store all tokens for the longest match

        for (size_t i = pos; i < input.length(); ++i) {
            string currChar(1, input[i]); // Convert char to string
            if (dfa.transitions.count(*currState) &&
                dfa.transitions.at(*currState).count(currChar)) {
                currState = &dfa.transitions.at(*currState).at(currChar);

                // If the current state is accepting
                if (dfa.accepting.count(*currState)) {
                    lastMatchPos = i + 1;
                    lastAcceptingState = currState;

                    // Collect tokens for the accepting state
                    const auto& acceptedTokens = dfa.accepting.at(*currState);
                    possibleTokens.assign(acceptedTokens.begin(), acceptedTokens.end());
                }
            } else {
                break; // Invalid transition
            }
        }

        // Handle errors (unrecognized sequences)
        if (possibleTokens.empty()) {
            // Log an error token and continue
            tokens.push_back("ERROR: Unrecognized sequence at position " + to_string(pos));
            ++pos; // Skip the problematic character and continue
            continue;
        }

        // Resolve ambiguities with priority if multiple tokens are possible
        string selectedToken = possibleTokens[0];
        if (possibleTokens.size() > 1) {
            size_t highestPriority = priority.size(); // Set the highest priority index initially
            for (const string& token : possibleTokens) {
                size_t index = find(priority.begin(), priority.end(), token) - priority.begin();
                if (index < highestPriority) {
                    highestPriority = index;
                    selectedToken = token;
                }
            }
        }

        // Add the selected token to the result
        tokens.push_back(selectedToken);

        // Move position to the end of the matched sequence
        pos = lastMatchPos;
    }

    return tokens;
}



// Function to write tokens to a file
void writeTokensToFile(const vector<string>& tokens, const string& filename) {
    ofstream outFile(filename);
    if (!outFile) {
        cerr << "Error: Unable to open file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    for (const string& token : tokens) {
        outFile << token << '\n';
    }

    outFile.close();
}


vector<string> extractPriority(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening rules file.\n";
        exit(1);
    }

    vector<string> keywords;  // Tokens from { ... } blocks
    vector<string> others;    // All other tokens in file order
    string line;

    while (getline(file, line)) {
        // Trim leading whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty()) continue;

        if (line.front() == '{') {
            line.pop_back();   // remove trailing }
            line.erase(0, 1);  // remove leading {
            stringstream ss(line);
            string keyword;
            while (ss >> keyword) {
                keywords.push_back(keyword); // store for front of priority list
            }
            continue;
        }

        if (line.front() == '[') {
            line.pop_back(); line.erase(0,1);
            stringstream ss(line);
            string symbol;
            while (ss >> symbol) {
                if (symbol.size() == 2 && symbol[0] == '\\')
                    symbol = symbol.substr(1);
                others.push_back(symbol);
            }
            continue;
        }

        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string name = line.substr(0, colonPos);
            name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());
            others.push_back(name);
            continue;
        }

    }

    vector<string> priority;
    // Change: only insert keywords **at the front**, preserving their order
    priority.insert(priority.end(), keywords.begin(), keywords.end());
    priority.insert(priority.end(), others.begin(), others.end());

    return priority;
}


int main() {
    DFA dfa = (nfa_to_dfa(filetoFSA()));
    dfa = minimizeDFA(dfa);
    writeDFATransitionsToFile(dfa,"minimized_dfa");
    // {"boolean","int","float","if","while","else",";",",","(",")","{","}","id","num","relop","assign","addop","mulop"};
    vector<string> priority = extractPriority("Rules");
    cout << "Token Priority Order:\n";
    for (const string& token : priority) {
        cout << token << "\n";
    }
    string inputFilename = "test.txt"; 
    ifstream inputFile(inputFilename);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << inputFilename << endl;
        return 1;
    }

    vector<string> tokens;
    string line;

    // Read file line by line
    while (getline(inputFile, line)) {
        vector<string> words = splitBywhitespace(line);
        for(string word :words){
          vector<string> wordToken =  tokenize(dfa,word,priority);
          for (const std::string& str : wordToken) {
            std::cout << str << " ";
    }
          
          tokens.insert(tokens.end(),wordToken.begin(),wordToken.end());}
        
    }

    inputFile.close();
    writeTokensToFile(tokens,"result.txt");
   print_dfa(dfa);

   

    return 0;
}

