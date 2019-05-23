#pragma once

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <map>
#include <string>

using std::vector;
using std::map;
using std::string;

void read_word(char *word, char *buf, uint64_t &c, uint64_t chunk_size, vector<map<string,uint64_t>> &buckets, int ranks);