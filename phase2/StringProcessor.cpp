#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

class StringProcessor {
 public:
  StringProcessor();
  virtual ~StringProcessor();
  vector<string> read_rules(string address);
  void skip_unnecessary_spaces(int& i, string rule_definition);
  string trim(const string& str);
  vector<string> string_processor(const string& input);
  string remove_backslash(string org);
};

StringProcessor::StringProcessor() {
  // ctor
}

StringProcessor::~StringProcessor() {
  // dtor
}

void StringProcessor::skip_unnecessary_spaces(int& i, string rule_definition) {
  while (i + 1 < (int)rule_definition.size() && rule_definition[i + 1] == ' ')
    i++;
}

vector<string> StringProcessor::read_rules(string address) {
  vector<string> lines;
  ifstream file(address);
  if (!file.is_open()) {
    cerr << "Error: Could not open the file: " << address << endl;
    return lines;
  }

  string line;
  while (getline(file, line)) {
    lines.push_back(line);
  }

  file.close();
  return lines;
}

string StringProcessor::remove_backslash(string org) {
  string to_remove_back_slash = "";
  if (org[0] == '\\' && org[1] == 'L')
    to_remove_back_slash = EPSILON;
  else {
    for (int i = 0; i < (int)org.size(); i++) {
      char c = org[i];
      if (!(c == '\\') || (i > 0 && org[i - 1] == '\\'))
        to_remove_back_slash += c;
    }
  }
  return to_remove_back_slash;
}

string StringProcessor::trim(const string& str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

vector<string> StringProcessor::string_processor(const string& input) {
  string trimmed_input = input;

  if ((trimmed_input.front() == '{' && trimmed_input.back() == '}') ||
      (trimmed_input.front() == '[' && trimmed_input.back() == ']')) {
    trimmed_input = trimmed_input.substr(1, trimmed_input.size() - 2);
  }
//  cout << trimmed_input << endl;
  vector<string> result;
  istringstream iss(trimmed_input);
  string token;

  while (iss >> token) {
    result.push_back(token);
  }

  return result;
}
