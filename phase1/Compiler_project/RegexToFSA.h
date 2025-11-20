#ifndef REGEXTOFSA_H
#define REGEXTOFSA_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
using namespace std;
struct NFS {
    map<string, map<string, vector<string>>> trans;
    vector<string> accepting;
    string starting;
};

NFS filetoFSA();

#endif
