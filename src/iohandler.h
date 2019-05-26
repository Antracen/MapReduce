#pragma once

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <map>
#include <string>

using std::vector;
using std::map;
using std::string;

void read_chunk(char *word, char *buf, unsigned long int chunk_size, vector<map<string,unsigned long int>> &buckets, int ranks);

