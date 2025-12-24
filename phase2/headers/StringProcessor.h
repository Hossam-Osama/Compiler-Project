#ifndef STRINGPROCESSOR_H
#define STRINGPROCESSOR_H

#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "State.h"

using namespace std;
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

#endif  // STRINGPROCESSOR_H
