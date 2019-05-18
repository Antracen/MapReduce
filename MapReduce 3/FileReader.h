#pragma once

#include <string>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <unordered_map>

using std::string;
using std::cout;
using std::endl;
using std::stringstream; 
using std::getline;
using std::unordered_map;

void readFile(char* name, int rank, unordered_map<string,int>& m);
