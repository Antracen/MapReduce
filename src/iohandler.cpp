#include "iohandler.h"

using std::map;
using std::unordered_map;
using std::vector;
using std::string;
using std::hash;

void read_chunk(char *word, char *buf, uint64_t chunk_size, vector<unordered_map<string,uint64_t>> &buckets, int ranks) {
	uint64_t c = 0;
	while(c < chunk_size) {
		bool word_overflow = false;
		int w = 0;
		while(!isalnum(buf[c]) && c < chunk_size) c++;

		while(c < chunk_size && (isalnum(buf[c]) || buf[c] == '\'')) {
			if(w + 1 > WORD_SIZE) {
				word_overflow = true;
				break;
			}
			word[w] = tolower(buf[c]);
			c++;
			w++;
		}
		if(word_overflow) continue;
		word[w] = '\0';

		if(w != 0 && w < WORD_SIZE) {
			hash<string> hasher;
			int h = hasher(word) % ranks;
			#pragma omp critical
			buckets[h][word] += 1;
		}
	}
}
