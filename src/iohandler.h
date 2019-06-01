#pragma once

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using std::vector;
using std::map;
using std::unordered_map;
using std::string;

void read_chunk(char *word, char *buf, uint64_t chunk_size, vector<unordered_map<string,uint64_t>> &buckets, int ranks);
