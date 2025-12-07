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
