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
vector<string> getProductionRules(vector<string> input, string startRule, map<pair<string, string>, string> parsingTable) ;
vector<string> readLexicalTokens() ;

#endif // PRODUCTION_RULES_H