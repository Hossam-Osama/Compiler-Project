#ifndef MAPPING_H
#define MAPPING_H

#include <string>
#include <vector>
#include <map>
#include <sstream>

// Function to split a string by spaces
std::vector<std::string> split(const std::string &str);

// Function to convert a vector of vectors to the desired map structure
void convertVectorToGrammar(const std::vector<std::vector<std::string>> &input, std::map<std::string, std::vector<std::vector<std::string>>> &grammar);

#endif // MAPPING_H
