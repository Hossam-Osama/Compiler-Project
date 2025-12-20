#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include "RegexToFSA.h"
using namespace std;
int state =0;
// struct NFS {
//     map<string, map<string, vector<string>>> trans;
//     vector<string> accepting;
//     string starting;

//     };

std::vector<std::string> splitByWhitespace(const std::string& str) {
    std::vector<std::string> result;
    std::istringstream stream(str);
    std::string word;
    while (stream >> word) {
        result.push_back(word);
    }
    return result;
}    
std::vector<std::string> tokenizeString(const std::string& input) {
    std::vector<std::string> result;
    std::string current;
    char prev = input[0];
    for (char ch : input) {
        // Check if the character is a delimiter
        if ((std::isspace(ch) || ch == '*' || ch == '|' || ch == '(' || ch == ')' || ch == '-'||ch == '+' )&&prev!='\\') {
            // If we have accumulated characters, add them to the result vector
            if (!current.empty()) {
               
                result.push_back(current);

                current.clear();
            }
            // Add the delimiter as a separate element
            
                result.push_back(std::string(1, ch));
            
        } else {
            // Accumulate characters
            current += ch;
        }
        prev=ch;
    }

    // Add any remaining characters as the last element
    if (!current.empty()) {
                result.push_back(current);
    }

    return result;
}



std::string ttrim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");  // Find the first non-whitespace character
    if (first == std::string::npos)  // If no non-whitespace character is found, return an empty string
        return "";

    size_t last = str.find_last_not_of(" \t\n\r\f\v");  // Find the last non-whitespace character
    return str.substr(first, last - first + 1);  // Return the substring containing the trimmed string
}

int findIndex(const std::vector<string>& vec, string target) {
    // Use std::find to find the iterator to the target
    auto it = find(vec.begin(), vec.end(), target);
    
    // If the iterator is not equal to vec.end(), the element was found
    if (it != vec.end()) {
        return distance(vec.begin(), it);  // Get the index using distance
    }
    
    return -1;  // Return -1 if element is not found
}
NFS handleDash(string x ,string y){
    NFS nfs;
    nfs.starting = to_string(state);
    nfs.accepting.push_back(to_string(state+1));
    nfs.trans[to_string(state)][x+"-"+y].push_back(to_string(state+1));
    nfs.trans[to_string(state+1)];
    state+=2;
    return nfs;


}
NFS charToNfs(string x){
    NFS nfs;
    nfs.starting = to_string(state);
    nfs.accepting.push_back(to_string(state+1));

    string symbol;

    // ✅ FIX: handle epsilon (\L)
    if (x == "\\L") {
        symbol = " ";   // epsilon
    }
    else if (x[0] == '\\') {
        symbol = x.substr(1);  // escaped literal
    }
    else {
        symbol = x;
    }

    nfs.trans[to_string(state)][symbol].push_back(to_string(state+1));
    nfs.trans[to_string(state+1)];

    state += 2;
    return nfs;
}
NFS  handleUnion(NFS nfs1,NFS nfs2){
    NFS nfs;
    nfs.starting = to_string(state);
    nfs.accepting.push_back(to_string(state+1));
    nfs.trans.insert(nfs1.trans.begin(),nfs1.trans.end());
    nfs.trans.insert(nfs2.trans.begin(),nfs2.trans.end());
    nfs.trans[to_string(state)][" "].push_back(nfs1.starting);
    nfs.trans[to_string(state)][" "].push_back(nfs2.starting);
    nfs.trans[nfs1.accepting[0]][" "].push_back(to_string(state+1));
    nfs.trans[nfs2.accepting[0]][" "].push_back(to_string(state+1));
    nfs.trans[to_string(state+1)];
    state+=2;
    return nfs;



  
}
NFS  handleConcat(NFS nfs1,NFS nfs2){
    NFS nfs;
    nfs.trans.insert(nfs1.trans.begin(),nfs1.trans.end());
    nfs.trans.insert(nfs2.trans.begin(),nfs2.trans.end());
    nfs.starting=nfs1.starting;
    nfs.accepting.push_back(nfs2.accepting[0]);
    nfs.trans[nfs1.accepting[0]][" "].push_back(nfs2.starting);
    return nfs;


  
}
NFS  handleKleeneClosure(NFS nfs1){
    NFS nfs;
    nfs.starting = to_string(state);
    nfs.accepting.push_back(to_string(state+1));
    nfs.trans.insert(nfs1.trans.begin(),nfs1.trans.end());
    nfs.trans[to_string(state)][" "].push_back(nfs1.starting);
    nfs.trans[to_string(state)][" "].push_back(to_string(state+1));
    nfs.trans[to_string(state+1)];
    nfs.trans[nfs1.accepting[0]][" "].push_back(to_string(state+1));
    nfs.trans[nfs1.accepting[0]][" "].push_back(nfs1.starting);
    state+=2;
    return nfs;




   


    

  
}
NFS  handlePositiveClosure(NFS nfs1){
    NFS nfs;
    nfs.starting = to_string(state);
    nfs.accepting.push_back(to_string(state+1));
    nfs.trans.insert(nfs1.trans.begin(),nfs1.trans.end());
    nfs.trans[to_string(state)][" "].push_back(nfs1.starting);
    nfs.trans[to_string(state+1)];
    nfs.trans[nfs1.accepting[0]][" "].push_back(to_string(state+1));
    nfs.trans[nfs1.accepting[0]][" "].push_back(nfs1.starting);
    state+=2;
    return nfs;
  
}
int findMatchingBracket(vector<string> & regexArray,int bracketIndex){
    int count =1;
    int i = bracketIndex;
    while(count!=0)
    {
        i++;
        if(regexArray[i]=="(")
            count++;
        
        else if (regexArray[i]==")")
            count--;
    }
    return i;

}
NFS changeNfsStatesName(NFS nfs){
    NFS result;
    result.starting=nfs.starting+"$";
    for(string state : nfs.accepting){
        result.accepting.emplace_back(state+"$");
    }
    for (auto& outerPair : nfs.trans) {

        // Add dot to the outer map's key
       string key  = outerPair.first + "$";
       map<string,vector<string>> value;

        // Traverse the inner map
        for (auto& innerPair : outerPair.second) {
            vector<string> vec;
            
            // Traverse the vector of strings
            for (auto& str : innerPair.second) {
                // Add dot to each string in the vector
                 vec.emplace_back(str+"$");
            }
           value.insert( {innerPair.first,vec});
        }
        result.trans.insert({key,value});
    }
    return result;
}
NFS regexToNFS(string regex , map<string,NFS>& def) {
     regex = ttrim(regex);
     vector<string> regexArray= tokenizeString(regex);
     map<string,NFS> tempMap;
     while(findIndex(regexArray,"-")!=-1){
        int dashIndex = findIndex(regexArray,"-");
        string newString =regexArray[dashIndex-1]+regexArray[dashIndex]+regexArray[dashIndex+1];
        tempMap.insert({newString,handleDash(regexArray[dashIndex-1],regexArray[dashIndex+1])});
        regexArray.erase(regexArray.begin()+ dashIndex-1,regexArray.begin()+dashIndex+2);
        regexArray.emplace(regexArray.begin()+ dashIndex-1,newString);

     }
      while(findIndex(regexArray,"(")!=-1){
        int bracketIndex = findIndex(regexArray,"(");
        int matchingBracketIndex = findMatchingBracket(regexArray,bracketIndex);
        string result;
        for (int i = bracketIndex+1;i<matchingBracketIndex;i++) 
        {
            result += regexArray[i];
            
        }
        tempMap.insert({result,regexToNFS(result,def)});
        regexArray.erase(regexArray.begin()+ bracketIndex,regexArray.begin()+matchingBracketIndex+1);
        regexArray.emplace(regexArray.begin()+ bracketIndex,result);



     }
     while(findIndex(regexArray,"*")!=-1){
        int starIndex = findIndex(regexArray,"*");
        string newString =regexArray[starIndex-1]+regexArray[starIndex];
        if (def.find(regexArray[starIndex-1]) != def.end()) 
        {  NFS renamedNfs =changeNfsStatesName(def[regexArray[starIndex-1]]);
          tempMap.insert({newString,handleKleeneClosure(renamedNfs)});
          def[regexArray[starIndex-1]]=renamedNfs;

          


        }
        else if (tempMap.find(regexArray[starIndex-1]) != tempMap.end())
        {
           tempMap.insert({newString,handleKleeneClosure(tempMap[regexArray[starIndex-1]])});
           
        }
        else
        {
           tempMap.insert({newString,handleKleeneClosure(charToNfs(regexArray[starIndex-1]))});
           

        }
         regexArray.erase(regexArray.begin()+ starIndex-1,regexArray.begin()+starIndex+1);
         regexArray.emplace(regexArray.begin()+ starIndex-1,newString);



     }
     while(findIndex(regexArray,"+")!=-1){
         int starIndex = findIndex(regexArray,"+");
        string newString =regexArray[starIndex-1]+regexArray[starIndex];
        if (def.find(regexArray[starIndex-1]) != def.end()) 
        {
          NFS renamedNfs =changeNfsStatesName(def[regexArray[starIndex-1]]);
          tempMap.insert({newString,handlePositiveClosure(renamedNfs)});
          def[regexArray[starIndex-1]]=renamedNfs;
          


        }
        else if (tempMap.find(regexArray[starIndex-1]) != tempMap.end())
        {
           tempMap.insert({newString,handlePositiveClosure(tempMap[regexArray[starIndex-1]])});
          
        }
        else
        {
           tempMap.insert({newString,handlePositiveClosure(charToNfs(regexArray[starIndex-1]))});
           

        }
         regexArray.erase(regexArray.begin()+ starIndex-1,regexArray.begin()+starIndex+1);
         regexArray.emplace(regexArray.begin()+ starIndex-1,newString);

     }
     //concatenation
     while(findIndex(regexArray," ")!=-1){
        int concatIndex = findIndex(regexArray," ");
        string newString =regexArray[concatIndex-1]+regexArray[concatIndex]+regexArray[concatIndex+1];
        NFS nfs1;
        NFS nfs2;
        if (def.find(regexArray[concatIndex-1]) != def.end()) 
        {//tempMap.insert({newString,handleUnion(def[regexArray[unionIndex-1]])});
        nfs1=def[regexArray[concatIndex-1]];
          NFS renamedNfs =changeNfsStatesName(def[regexArray[concatIndex-1]]);
          def[regexArray[concatIndex-1]]=renamedNfs;


        }
        else if (tempMap.find(regexArray[concatIndex-1]) != tempMap.end())
        { 
           nfs1=tempMap[regexArray[concatIndex-1]];
           
        }
        else
        {
           nfs1=charToNfs(regexArray[concatIndex-1]);
        }

        if (def.find(regexArray[concatIndex+1]) != def.end()) 
        {//tempMap.insert({newString,handleUnion(def[regexArray[unionIndex-1]])});
        
          nfs2=def[regexArray[concatIndex+1]];
          NFS renamedNfs =changeNfsStatesName(def[regexArray[concatIndex+1]]);
          def[regexArray[concatIndex+1]]=renamedNfs;
        }
        else if (tempMap.find(regexArray[concatIndex+1]) != tempMap.end())
        { 
           nfs2=tempMap[regexArray[concatIndex+1]];
           //tempMap.erase(regexArray[unionIndex+1]);
        }
        else
        {
           nfs2=charToNfs(regexArray[concatIndex+1]);
        }
        tempMap[newString]=handleConcat(nfs1,nfs2);
         regexArray.erase(regexArray.begin()+ concatIndex-1,regexArray.begin()+concatIndex+2);
         regexArray.emplace(regexArray.begin()+ concatIndex-1,newString);

     }

     while(findIndex(regexArray,"|")!=-1){
        int unionIndex = findIndex(regexArray,"|");
        string newString =regexArray[unionIndex-1]+regexArray[unionIndex]+regexArray[unionIndex+1];
        NFS nfs1;
        NFS nfs2;
        if (def.find(regexArray[unionIndex-1]) != def.end()) 
        {//tempMap.insert({newString,handleUnion(def[regexArray[unionIndex-1]])});
           nfs1=def[regexArray[unionIndex-1]];
          NFS renamedNfs =changeNfsStatesName(def[regexArray[unionIndex-1]]);
          def[regexArray[unionIndex-1]]=renamedNfs;
        }
        else if (tempMap.find(regexArray[unionIndex-1]) != tempMap.end())
        { 
           nfs1=tempMap[regexArray[unionIndex-1]];
           
        }
        else
        {
           nfs1=charToNfs(regexArray[unionIndex-1]);
        }

        if (def.find(regexArray[unionIndex+1]) != def.end()) 
        {//tempMap.insert({newString,handleUnion(def[regexArray[unionIndex-1]])});
          nfs2=def[regexArray[unionIndex+1]];
          NFS renamedNfs =changeNfsStatesName(def[regexArray[unionIndex+1]]);
          def[regexArray[unionIndex+1]]=renamedNfs;
        }
        else if (tempMap.find(regexArray[unionIndex+1]) != tempMap.end())
        { 
           nfs2=tempMap[regexArray[unionIndex+1]];
           //tempMap.erase(regexArray[unionIndex+1]);
        }
        else
        {
           nfs2=charToNfs(regexArray[unionIndex+1]);
        }
        tempMap[newString]=handleUnion(nfs1,nfs2);
         regexArray.erase(regexArray.begin()+ unionIndex-1,regexArray.begin()+unionIndex+2);
         regexArray.emplace(regexArray.begin()+ unionIndex-1,newString);


     }
     
     if(tempMap.find(regexArray[0]) != tempMap.end())
        return tempMap[regexArray[0]];
     else{
        return charToNfs(regexArray[0]);
     } 
     




    


}
void print(NFS nfs)  {
        // Print starting state
        std::cout << "Starting State: " << nfs.starting << "\n";

        // Print accepting states
        std::cout << "Accepting States: ";
        for (const auto& state : nfs.accepting) {
            std::cout << state << " ";
        }
        std::cout << "\n";

        // Print transitions
        std::cout << "Transitions:\n";
        for (const auto& [fromState, innerMap] : nfs.trans) {
            std::cout << "  From State: " << fromState << "\n";
            for (const auto& [symbol, toStates] : innerMap) {
                std::cout << "    On Symbol '" << symbol << "' -> ";
                for (const auto& toState : toStates) {
                    std::cout << toState << " ";
                }
                std::cout << "\n";
            }
        }
    }

    NFS expandRangeKeys(NFS nfs) {
    NFS newNFS;
    newNFS.starting = nfs.starting;
    newNFS.accepting = nfs.accepting;
    newNFS.trans.clear(); // Clear the trans map to populate it with updated keys

    for (const auto& [outerKey, innerMap] : nfs.trans) {
        map<string, vector<string>> expandedInnerMap;

        for (const auto& [innerKey, values] : innerMap) {
            size_t dashPos = innerKey.find('-');
            if (dashPos != string::npos && dashPos > 0 && dashPos < innerKey.length() - 1) {
                char startChar = innerKey[dashPos - 1];
                char endChar = innerKey[dashPos + 1];

                // Ensure the range is valid
                if (startChar < endChar) {
                    for (char c = startChar; c <= endChar; ++c) {
                        string expandedKey(1, c);
                        expandedInnerMap[expandedKey] = values; // Add expanded key with same values
                    }
                }
            } else {
                // If no range, copy the key as-is
                expandedInnerMap[innerKey] = values;
            }
        }

        // Add the updated inner map to the new outer map
        newNFS.trans[outerKey] = expandedInnerMap;
    }

    return newNFS;
}



NFS filetoFSA(){
    string Line;
    ifstream MyReadFile("Rules");
    map<string,NFS> def;
    map<string,NFS> exp;

    while (getline (MyReadFile, Line)){
        if(Line.find(":")!=string::npos)
        {
            exp.insert({Line.substr(0,Line.find(":")),regexToNFS(Line.substr(Line.find(":")+1,Line.size()),def)});  
        }
        else if(Line.find(" =")!=string::npos)
        {
            def.insert({Line.substr(0,Line.find(" =")),regexToNFS(Line.substr(Line.find(" =")+2,Line.size()),def)});
        }
        else if(Line[0]=='{'||Line[0]=='['){
            vector<string> keyWords = splitByWhitespace(ttrim(Line.substr(1,Line.length()-2)));
            for (string keyWord : keyWords) {
                if(keyWord[0]=='\\')
                keyWord=keyWord.substr(1);
                NFS nfs=charToNfs(string(1,keyWord[0]));
                for(int i =1;i<keyWord.length();i++){
                    nfs=handleConcat(nfs,charToNfs(string(1,keyWord[i])));
                }
                exp.insert({keyWord,nfs});

            }



        }
    }

    NFS finalNfs;
    finalNfs.starting=to_string(state);
    for (std::map<std::string, NFS>::iterator it = exp.begin(); it != exp.end();++it) {
        finalNfs.trans.insert(it->second.trans.begin(),it->second.trans.end());
        finalNfs.accepting.emplace_back(it->second.accepting[0]+" "+it->first);
        finalNfs.trans[to_string(state)][" "].emplace_back(it->second.starting);
    }
    def.clear();
    exp.clear();
    finalNfs = expandRangeKeys(finalNfs);
    return finalNfs;
    

    
}



// int main(){
//     print((filetoFSA()));
//     return 0;



// }











