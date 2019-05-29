#include "iohandler.h"

using std::map;
using std::unordered_map;
using std::vector;
using std::string;
using std::hash;

void read_chunk(char *word, char *buf, unsigned long int chunk_size, vector<unordered_map<string,unsigned long int>> &buckets, int ranks) {
    unsigned long int c = 0;
    while(c < chunk_size) {
        int w = 0;
        while(!isalnum(buf[c]) && c < chunk_size) c++;

        while(c < chunk_size && (isalnum(buf[c]) || buf[c] == '\'')) {
            word[w] = tolower(buf[c]);
            c++;
            w++;
        }
        word[w] = '\0';

        if(w != 0 && w < WORD_SIZE) {
            hash<string> hasher;
            int h = hasher(word) % ranks;
            #pragma omp critical
			buckets[h][word] += 1;
        }
    }
}
