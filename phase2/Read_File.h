#ifndef READ_FILE_H
#define READ_FILE_H

#include <string>
#include <vector>

// Function to trim whitespace and replace newlines with spaces in a string
std::string trim(const std::string &str);

// Function to split a string by a delimiter of more than one character
std::vector<std::string> split(const std::string &str, const std::string &delimiter);

// Function to process a file and return a vector of parsed data
std::vector<std::vector<std::string>> processFile(const std::string &fileName);

#endif // FILE_PROCESSOR_H
