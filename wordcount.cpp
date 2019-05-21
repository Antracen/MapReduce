// TODO:
	// Free memory when not needed anymore
	// Make sure it handles running on only one process
	// Make sure it handles running on "too many" processes (some get no chunks)
	// Make sure it handles no chunks (only extra chunk)
	// Do we need a more explicit "reduce" call?
	// Can we utilize OpenMP?
	// Can we utilize operations such as gather, alltoall, scatter etc?
	// Which variables should be uint64_t?
#define TOO_FEW_ARGUMENTS 007
#define NONEXISTENT_FILE 1919
#define CHUNK_SIZE 64000000

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
#include <functional>
#include "commhandler.h"
#include "iohandler.h"

using std::map;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::hash;

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	double start_time = MPI_Wtime();

	int res;

	char *filename;

	/* Parse command line arguments */
		if(argc < 2) {
			cout << "Too few arguments. Aborting." << endl;
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
			cout << "File does not exist. Aborting." << endl;		
			MPI_Abort(MPI_COMM_WORLD, NONEXISTENT_FILE);
		}

	/* Calculate what to read */
		MPI_Offset file_size;
		MPI_File_get_size(f, &file_size);
		int total_chunks = file_size / CHUNK_SIZE;
		vector<int> chunks_to_read;
		for(int i = rank; i < total_chunks; i++) if(i % ranks == rank) chunks_to_read.push_back(i);
		uint64_t extra_chunk = file_size % CHUNK_SIZE;

		if(rank == 0) cout << "File size: " << file_size << endl << "Total chunks: " << total_chunks << endl;

		if(rank != 0) cout << "Rank " << rank << " reads " << chunks_to_read.size() << " chunks " << endl;
		else cout << "Master reads " << chunks_to_read.size() << " chunks and " <<  extra_chunk << " extra data " << endl;

	/* Map the chunks into KV pairs */
		vector<map<string, uint64_t>> buckets(ranks);
		char *buf = (char*) malloc(CHUNK_SIZE + 1);
		char *word = (char*) malloc(CHUNK_SIZE + 1);
		//%#OMP?
		for(int chunk : chunks_to_read) {
			MPI_File_read_at(f, chunk*CHUNK_SIZE, buf, CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
			buf[CHUNK_SIZE] = '\0';
			uint64_t c = 0;
			while(c < CHUNK_SIZE) read_word(word, buf, c, CHUNK_SIZE, buckets, ranks);
		}

		if(extra_chunk != 0 && rank == 0) {
			cout << "Reading extra big chungus. Time: " << (MPI_Wtime() - start_time) << endl;			
			MPI_File_read_at(f, file_size - extra_chunk, buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
			cout << "Chungus read. Time: " << (MPI_Wtime() - start_time) << endl;			
			buf[extra_chunk] = '\0';
			uint64_t c = 0;
			while(c < extra_chunk) read_word(word, buf, c, extra_chunk, buckets, ranks);
		}

		cout << "Map phase done. Rank: " << rank << " time: " << (MPI_Wtime() - start_time) << endl;

	/* Send the data to each owning process */
		// Calculate (1) how much to send to everyone and (2) how much I will receive
		int *receive_amount = new int[ranks]; // How much will I receive in total
		for(int i = 0; i < ranks; i++){ 
			int num_words = buckets[i].size();
			MPI_Allreduce(&num_words, &receive_amount[i], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		}

		// Send the words to their rightful owner
		for(int i = 0; i < ranks; i++) send_words(buckets[i],i);

		cout << "Sent all data " << (MPI_Wtime() - start_time) << endl;
        

		map<string,uint64_t> bucket; // All my worlds
		// Receive my words that the other guys had
		int amount = receive_amount[rank]; // How much I should receive
		receive_words(bucket, amount); 

		cout << "Received my data " << (MPI_Wtime() - start_time) << endl;

	/* Other guys send their parts while master gets everything and prints */
		int bucket_size = bucket.size();
		MPI_Reduce(&bucket_size, &amount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		amount = amount - bucket_size;
		if(rank != 0) {
			send_words(bucket,0);
		} else {		
			// Receive words
			receive_words(bucket, amount);

			cout << "Master has all words " << (MPI_Wtime() - start_time) << endl;
			for(auto &p : bucket) cout << "(" << p.first << "," << p.second << ") in 0 (final count[down])" << endl;
			double time = MPI_Wtime() - start_time;
			cout << "Time: " << time << endl;
		}
	MPI_Finalize();
}
