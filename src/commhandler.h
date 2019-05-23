#pragma once
#include <iostream>
#include <map>
#include <mpi.h>
#include <string>

using std::map; 
using std::cout; 
using std::endl; 
using std::string;

void receive_words(map<string, uint64_t> &bucket, int amount);
void send_words(map<string, uint64_t> &bucket, int receiver);