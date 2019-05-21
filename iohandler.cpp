#include "iohandler.h"
#include <cstdint>
#include <cctype>
#include <vector>
#include <map>

using std::map;
using std::vector;
using std::string;
using std::hash;

void read_word(char* word, char *buf, uint64_t &c, uint64_t chunk_size, vector<map<string,uint64_t>> &buckets, int ranks) {
    int w = 0;
	while(!isalnum(buf[c]) && c < chunk_size) c++;

    while(c < chunk_size && (isalnum(buf[c]) || buf[c] == '\'')) {
	    word[w] = tolower(buf[c]);
		c++;
		w++;
	}
	word[w] = '\0';

    if(w != 0) {
        hash<string> hasher;
        int h = hasher(word) % ranks;
        buckets[h][word] = (buckets[h].count(word)) ? buckets[h][word] + 1 : 1;
    }
}