#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring> // For strcmp

// Function to compare a string with keys in a map using strcmp
bool compareStringToKeyByStrCmp(const std::map<std::string, std::set<std::string>>& myMap, const std::string& key);

// Function to split a string by a delimiter and return tokens
std::vector<std::string> split(const std::string &str, char delimiter);

// Function to tokenize a production rule string into components
std::vector<std::string> tokenize(const std::string &production);

// Function to calculate the First set for the grammar
void calculateFirst(std::map<std::string, std::vector<std::vector<std::string>>>& grammar, 
                    std::map<std::string, std::set<std::string>>& first);

// Function to calculate the Follow set for the grammar
void calculateFollow(std::map<std::string, std::vector<std::vector<std::string>>>& grammar,
                     std::map<std::string, std::set<std::string>>& first, 
                     std::map<std::string, std::set<std::string>>& follow, 
                     const std::string& startSymbol);

#endif // GRAMMAR_UTILS_H
