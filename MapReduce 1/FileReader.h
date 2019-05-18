#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <mpi.h>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <vector>

using std::string;
using std::cout;
using std::endl;
using std::stringstream; 
using std::getline;
using std::unordered_map;
using std::regex;
using std::regex_replace;
using std::vector;


void readFile(char* name, int rank, int ranks, vector<unordered_map<string,int>>& buckets);
