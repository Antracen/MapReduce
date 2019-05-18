#define TOO_FEW_ARGUMENTS 007
#define NONEXISTENT_FILE 1919
// #define CHUNK_SIZE 64000000
#define CHUNK_SIZE 1000

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <string>
#include <regex>
#include <algorithm>

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	int res;

	char *filename;

	/* Parse command line arguments */
		if(argc < 2) {
			std::cout << "Too few arguments. Aborting." << std::endl;
			MPI_Abort(MPI_COMM_WORLD, TOO_FEW_ARGUMENTS);
		}
		else filename = argv[1];

	/* Get rank info */
		int rank, ranks;
		MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		MPI_Comm_size(MPI_COMM_WORLD,&ranks);

	/* Open file */
		MPI_File f;
		res = MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

		if(res) {
			std::cout << "File does not exist. Aborting." << std::endl;		
			MPI_Abort(MPI_COMM_WORLD, NONEXISTENT_FILE);
		}

	/* Calculate what to read */
		MPI_Offset file_size;
		MPI_File_get_size(f, &file_size);
		int chunks_per_process = file_size / CHUNK_SIZE / ranks;
		int extra_chunk = file_size % CHUNK_SIZE;

	/* Handle all chunks */
		std::vector<std::map<std::string, uint64_t>> buckets(ranks);
		char *buf = (char*) malloc(CHUNK_SIZE + 1);
		char *word = (char*) malloc(CHUNK_SIZE + 1);
		for(int i = 0; i < chunks_per_process; i++) {
			MPI_File_read_at(f, rank*CHUNK_SIZE*chunks_per_process + i*CHUNK_SIZE, buf, CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
			buf[CHUNK_SIZE] = '\0';
			int c = 0;
			while(c < CHUNK_SIZE) {
				int w = 0;
				while(!isalpha(buf[c]) && c < CHUNK_SIZE) c++;

				while(c < CHUNK_SIZE && (isalpha(buf[c]) || buf[c] == '\'')) {
					word[w] = tolower(buf[c]);
					c++;
					w++;
				}
				word[w] = '\0';
		
				if(w != 0) {
					std::hash<std::string> hasher;
					int h = hasher(word) % ranks;
					buckets[h][word] = (buckets[h].count(word)) ? buckets[h][word] + 1 : 1;
				}
			}
		}

		if(extra_chunk != 0 && rank == 0) {
			MPI_File_read_at(f, ranks*CHUNK_SIZE*chunks_per_process, buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
			buf[extra_chunk] = '\0';
			int c = 0;
			while(c < extra_chunk) {
				int w = 0;
				while(!isalpha(buf[c]) && c < extra_chunk) c++;

				while(c < extra_chunk && (isalpha(buf[c]) || buf[c] == '\'')) {
					word[w] = tolower(buf[c]);
					c++;
					w++;
				}
				word[w] = '\0';
				if(w != 0) {
					std::hash<std::string> hasher;
					int h = hasher(word) % ranks;
					buckets[h][word] = (buckets[h].count(word)) ? buckets[h][word] + 1 : 1; 
				}
			}
		}

		for(int i = 0; i < ranks; i++) {
			for(auto &pair : buckets[i]) {
				std::cout << "Rank " << rank << " Bucket " << i << " : " << pair.first << " count:" << pair.second << std::endl;
			}
		}

	MPI_Finalize();
}
