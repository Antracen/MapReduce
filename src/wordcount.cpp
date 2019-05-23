// TODO:
	// Free memory when not needed anymore
	// Do we need a more explicit "reduce" call?
	// Can we utilize OpenMP?
	// Can we utilize operations such as gather, alltoall, scatter etc?
	// Which variables should be uint64_t?
	// Can we use Ireduce?
#define TOO_FEW_ARGUMENTS 007
#define NONEXISTENT_FILE 1919
#define CHUNK_SIZE 64000000

// #define DEBUG
#define COUNTSORT

#include "commhandler.h"
#include "iohandler.h"

#include <mpi.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

using std::map;		
using std::string;	
using std::cout;
using std::endl;	
using std::vector;	
using std::pair;
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

		#ifdef DEBUG
			if(rank == 0) cout << "File size: " << file_size << endl << "Total chunks: " << total_chunks << endl;

			if(rank != 0) cout << "Rank " << rank << " reads " << chunks_to_read.size() << " chunks " << endl;
			else cout << "Master reads " << chunks_to_read.size() << " chunks and " <<  extra_chunk << " extra data " << endl;
		#endif

	/* Map the chunks into KV pairs */
		vector<vector<map<string, uint64_t>>> all_buckets;

		//%#OMP?
		#pragma omp parallel for
		for(size_t i = 0; i < chunks_to_read.size(); i++) {
			int chunk = chunks_to_read[i];
			vector<map<string, uint64_t>> buckets(ranks);
			char *buf = (char*) malloc(CHUNK_SIZE + 1);
			char *word = (char*) malloc(CHUNK_SIZE + 1);
			MPI_File_read_at(f, chunk*CHUNK_SIZE, buf, CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
			buf[CHUNK_SIZE] = '\0';
			uint64_t c = 0;
			while(c < CHUNK_SIZE) read_word(word, buf, c, CHUNK_SIZE, buckets, ranks);
			all_buckets.push_back(buckets);
		}

		if(extra_chunk != 0 && rank == 0) {
			vector<map<string, uint64_t>> buckets(ranks);
			char *buf = (char*) malloc(CHUNK_SIZE + 1);
			char *word = (char*) malloc(CHUNK_SIZE + 1);
			#ifdef DEBUG
				cout << "Reading extra big chungus. Time: " << (MPI_Wtime() - start_time) << endl;			
			#endif
			MPI_File_read_at(f, file_size - extra_chunk, buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
			#ifdef DEBUG
				cout << "Chungus read. Time: " << (MPI_Wtime() - start_time) << endl;			
			#endif
			buf[extra_chunk] = '\0';
			uint64_t c = 0;
			while(c < extra_chunk) read_word(word, buf, c, extra_chunk, buckets, ranks);
			all_buckets.push_back(buckets);
		}

		#ifdef DEBUG
			cout << "Map phase done. Rank: " << rank << " time: " << (MPI_Wtime() - start_time) << endl;
		#endif

	/* Send the data to each owning process */
		// Calculate (1) how much to send to everyone and (2) how much I will receive
		//int *receive_amount = new int[ranks]; // How much will I receive in total
		int amount;

		for(int i = 0; i < ranks; i++) { 
			int num_words = 0;
			for(size_t j = 0; j < all_buckets.size(); j++) num_words += all_buckets[j][i].size();
			#ifdef DEBUG
				cout << "REDUCING in rank " << rank << endl;
			#endif
			MPI_Reduce(&num_words,&amount,1,MPI_INT,MPI_SUM,i,MPI_COMM_WORLD);
		}

		// Send the words to their rightful owner
		#ifdef DEBUG
			cout << "SENDING WORDS in rank " << rank << endl;
		#endif
		for(size_t j = 0; j < all_buckets.size(); j++) {
			for(int i = 0; i < ranks; i++) send_words(all_buckets[j][i],i);
		}

		#ifdef DEBUG
			cout << "Sent all data in rank " << rank << " " << (MPI_Wtime() - start_time) << endl;
		#endif
		map<string,uint64_t> bucket; // All my worlds
		// Receive my words that the other guys had
		//int amount = receive_amount[rank]; // How much I should receive

		#ifdef DEBUG
			cout << "Receiving words in rank " << rank << " " << (MPI_Wtime() - start_time) << endl;
		#endif
		receive_words(bucket, amount); 
		#ifdef DEBUG
			cout << "Received all words in rank " << rank << " " << (MPI_Wtime() - start_time) << endl;
		#endif

	/* Other guys send their parts while master gets everything and prints */
		int bucket_size = bucket.size();
		#ifdef DEBUG
			cout << "Calculating how many words master should get (rank " << rank << ") "<< (MPI_Wtime() - start_time) << endl;
		#endif
		MPI_Reduce(&bucket_size, &amount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		#ifdef DEBUG
			cout << "Done calculating how many words master should get " << (MPI_Wtime() - start_time) << endl;
		#endif
		amount = amount - bucket_size;
		if(rank != 0) {
			send_words(bucket,0);
		} else {	
			#ifdef DEBUG
				cout << "Master is receiving words " << (MPI_Wtime() - start_time) << endl;
			#endif	
			// Receive words
			receive_words(bucket, amount);

			#ifdef DEBUG
				cout << "Master has all words " << (MPI_Wtime() - start_time) << endl;
			#endif
			#ifdef COUNTSORT
				vector<pair<string,int>> wordsToSort; 
				for(auto &p : bucket) wordsToSort.emplace_back(p.first,p.second); 
				sort(wordsToSort.begin(),wordsToSort.end(),[](pair<string,int> &e1, pair<string,int> &e2){return e1.second < e2.second;}); 
				for(auto &p : wordsToSort) cout << "(" << p.first << "," << p.second << ")" << endl; 
				double time = MPI_Wtime() - start_time;
				cout << "Time: " << time << endl;
			#else
				for(auto &p : bucket) cout << "(" << p.first << "," << p.second << ") in 0 (final count[down])" << endl;
				double time = MPI_Wtime() - start_time;
				cout << "Time: " << time << endl;
			#endif
		} 

	/* Free memory and finalize */
		// free(buf);
		// free(word);
		MPI_Finalize();
}
