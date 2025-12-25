#ifndef PRODUCTION_RULES_H
#define PRODUCTION_RULES_H
#include<iostream>
#include <fstream>
#include<string>
#include<sstream>
#include <vector>
#include <stdexcept>
#include <map>
#include "LL1ParsingTableGenerator.h"

vector<string> splitRulesString(const string& str, const string& delimiter) ;
vector<string> getProductionRules(vector<string>& input, string& startRule, LL1ParsingTableGenerator& parsingTable) ;
vector<string> readLexicalTokens() ;
void printResults(const vector<string>& stack, string fileName) ;
#endif // PRODUCTION_RULES_H