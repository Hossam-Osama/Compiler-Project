#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "../headers/Read_File.h"

// Helper function to trim whitespace and replace newlines with spaces in a string
std::string trim2(const std::string &str) {
    size_t first = str.find_first_not_of(" \n\t\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \n\t\r");
    std::string trimmed = str.substr(first, last - first + 1);
    std::replace(trimmed.begin(), trimmed.end(), '\n', ' '); // Replace newlines with spaces
    return trimmed;
}

// Function to split a string by a delimiter of more than one character
std::vector<std::string> split(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    return tokens;
}

std::vector<std::vector<std::string>> processFile(const std::string &fileName) {
    // Step 1: Read the contents of a text file into a string
    std::ifstream file(fileName);
    if (!file) {
        throw std::runtime_error("Error: Unable to open file.");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();

    // Step 2: Split the string into a vector of strings
    std::vector<std::string> elements = split(fileContent, "#"); // Adjust delimiter as needed

    // Step 3: Remove the first element
    if (!elements.empty()) {
        elements.erase(elements.begin());
    }

    // Step 4: Trim each element
    for (auto &elem : elements) {
        elem = trim2(elem);
    }

     // Step 5: Process each element by splitting into " =" parts and then by "|"
    std::vector<std::vector<std::string>> result;
    for (auto &elem : elements) {
        std::replace(elem.begin(), elem.end(), '\n', ' '); // Replace newlines with spaces in the string
        std::vector<std::string> keyValue = split(elem, " =");
        if (keyValue.size() == 2) {
            std::vector<std::string> combined;
            combined.push_back(trim2(keyValue[0])); // Add the key
            std::vector<std::string> valueParts = split(keyValue[1], "|");
            for (auto &part : valueParts) {
                combined.push_back(trim2(part)); // Add the trimmed value parts
            }
            result.push_back(combined);
        } else {
            throw std::runtime_error("Warning: Element does not contain exactly one '='");
        }
    }

    return result;
}

// int main() {
//     try {
//         std::vector<std::vector<std::string>> processedData = processFile("grammer.txt");
//         std::cout << "Processed Data Structure:" << std::endl;
//         std::cout << "[" << std::endl;
//         for (const auto &valueParts : processedData) {
//             std::cout << "  [";
//             for (size_t i = 0; i < valueParts.size(); ++i) {
//                 std::cout << "\"" << valueParts[i] << "\"";
//                 if (i < valueParts.size() - 1) std::cout << ", ";
//             }
//             std::cout << "]," << std::endl;
//         }
//         std::cout << "]" << std::endl;
//     } catch (const std::exception &e) {
//         std::cerr << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }

