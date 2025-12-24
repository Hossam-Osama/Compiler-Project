#ifndef STATE_H
#define STATE_H
#include <iostream>
#include <map>
#include <set>
#include <vector>

#define EPSILON '\0'

using namespace std;
class State {
 public:
  // May add the name of the ReGeX the state is accepting
  State();
  virtual ~State();
  bool is_accepted = false;
  bool is_invalid = false;
  map<char, vector<State*>> transitions;
  // Variable string to indicate to which it's accepting
  string accepted_rule;
  
  void add_transition(char action, State* to_state);
  void print_recursive(set<const State*>& visited) const;
  void print_state_info() const;
};

#endif  // STATE_H
