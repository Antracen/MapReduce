// TODO:
	// Free memory when not needed anymore AS SOON AS A VARIABLE IS NOT NEEDED IT SHOULD BE free:d / delete:d
	// Do we need a more explicit "reduce" call?
	// Can we utilize OpenMP further?
	// Can we utilize operations such as gather, alltoall, scatter etc?
	// Which variables should be uint64_t?
	// Can we use Ireduce?
	// Can we utilize padding?
	// Should we have an unordered map?
	// Make sure we free the memory allocated in Isend in the commhandler
	// Check if the results are actually correct.
	// char* word = (char*) malloc(buf_size); could maybe be changed so it only allocates WORD_SIZE*num_chunks_local or something
	// When we send function(new int[2] {2,1}) does the array get deleted afterwards?

#define TOO_FEW_ARGUMENTS 007
#define NONEXISTENT_FILE 1919

#include "iohandler.h"

#include <mpi.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <unordered_map>

using std::map;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::pair;
using std::hash;
using std::max;
using std::ofstream;
using std::streambuf;

class Message {
	public:
		uint64_t count; // Should this be private?
		char word[WORD_SIZE]; // Should this be private?

		Message(uint64_t& c,const char *w): count(c) { strcpy(word,w); }
		Message(){}
		~Message(){}
};

int main(int argc, char *argv[]){
	/* Initialize */
		MPI_Init(&argc,&argv);
		double start_time = MPI_Wtime();
		int res;
		ofstream ofs; 
		uint64_t chunk_size = CHUNK_SIZE;
		uint64_t word_size = WORD_SIZE;
		uint64_t max_concurrent_chunks = MAX_CONCURRENT_CHUNKS;

	/* Parse command line arguments */
		if(argc < 3) {
			cout << "Too few arguments. Aborting." << endl;
			MPI_Abort(MPI_COMM_WORLD, TOO_FEW_ARGUMENTS);
		}

	/* Get rank info */
		int rank, ranks;
		MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		MPI_Comm_size(MPI_COMM_WORLD,&ranks);
		uint64_t bigrank = rank;
		uint64_t bigranks = ranks;

	/* Open file */
		if(rank == 0) ofs.open(argv[2], std::ofstream::out);
		MPI_File f;
		res = MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &f);

		if(res) {
			cout << "File does not exist. Aborting." << endl;
			MPI_Abort(MPI_COMM_WORLD, NONEXISTENT_FILE);
		}

	/* Create Message struct */
		MPI_Datatype message_struct;
		MPI_Type_create_struct(2, new int[2] {1,WORD_SIZE}, new MPI_Aint[2]{0,sizeof(uint64_t)}, new MPI_Datatype[2]{MPI_UINT64_T,MPI_CHAR}, &message_struct);
		MPI_Type_commit(&message_struct);

	/* Calculate what to read */
		MPI_Offset file_size;
		MPI_File_get_size(f, &file_size);

		uint64_t num_chunks_total = file_size / chunk_size;
		uint64_t num_chunks_local = num_chunks_total / bigranks;
		uint64_t extra_chunk = (bigrank < num_chunks_total % bigranks);
		uint64_t extra_bytes = (rank == ranks-1) * (file_size % chunk_size);

		#ifdef DEBUG
			cout << "RANK " << rank << " will read number of chunks: " << num_chunks_local << endl;
		#endif

	/* Map the chunks into KV pairs */
		vector<unordered_map<string, uint64_t>> buckets(ranks);

		uint64_t buf_size = chunk_size*max_concurrent_chunks;
		uint64_t word_buf_size = (word_size + 1)*max_concurrent_chunks;
		char* buf = new char[buf_size];
		char* word = new char[word_buf_size];

		MPI_Datatype read_type, chunk_type;
		MPI_Type_contiguous(chunk_size, MPI_CHAR, &chunk_type);
		MPI_Type_create_resized(chunk_type, 0, chunk_size*bigranks, &read_type);
		MPI_Type_commit(&chunk_type);
		MPI_Type_commit(&read_type);

		MPI_File_set_view(f, chunk_size*bigrank, chunk_type, read_type, "native", MPI_INFO_NULL);

		uint64_t chunks_left = num_chunks_local;
		uint64_t chunks_to_read = 0;
		uint64_t chunk_pos = 0;

		while(chunks_left > 0) {
			if(chunks_left >= MAX_CONCURRENT_CHUNKS) chunks_to_read = MAX_CONCURRENT_CHUNKS;
            else chunks_to_read = chunks_left;
			MPI_File_read_all(f, buf, chunks_to_read, chunk_type, MPI_STATUS_IGNORE);
			#pragma omp parallel for
			for(uint64_t i = 0; i < chunks_to_read; i++) {
				read_chunk(&word[i*(word_size+1)], &buf[i*chunk_size], chunk_size, buckets, ranks);
			}
			chunks_left -= chunks_to_read;
            chunk_pos += chunks_to_read;
		}

		if(extra_chunk) {
			MPI_File_read(f, buf, 1, chunk_type, MPI_STATUS_IGNORE);
			read_chunk(word, buf, chunk_size, buckets, ranks);
		}

		if(extra_bytes > 0) {
			MPI_File_read(f, buf, 1, chunk_type, MPI_STATUS_IGNORE);
            read_chunk(word, buf, extra_chunk, buckets, ranks);
		}

		delete[](buf);
		delete[](word);

		#ifdef DEBUG
			cout << "Map phase done. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
		#endif

	/* Redistribute the data into each process who should handle it */
		int *amounts = new int[ranks]; // How much I receive from every process
		int *local_amounts = new int[ranks]; // How much I send to every process
		int send_amount; // How many messages will I send in total?
		for(int i = 0; i < ranks; i++) local_amounts[i] = buckets[i].size();
		MPI_Request r; 
		MPI_Ialltoall(local_amounts,1,MPI_INT,amounts,1,MPI_INT,MPI_COMM_WORLD,&r);

		int recv_amount = 0;
		int *send_displacements = new int[ranks];
		send_displacements[0] = 0;
		send_amount = local_amounts[0];
		for(int i = 1; i < ranks; i++) {
			//local_amounts[i] = buckets[i].size();
			send_amount += local_amounts[i];
			send_displacements[i] = send_displacements[i-1] + local_amounts[i-1];
		}

		//MPI_Alltoall(local_amounts, 1, MPI_INT, amounts, 1, MPI_INT, MPI_COMM_WORLD);
		
		Message *send_buckets = new Message[send_amount];
		int j = 0;
		for(int i = 0; i < ranks; i++) {
			for(auto &p : buckets[i]) {
				send_buckets[j] = Message(p.second, p.first.c_str());
				j++;
			}
		}

		int *recv_displacements = new int[ranks];
		recv_displacements[0] = 0;

		
		MPI_Wait(&r,MPI_STATUS_IGNORE); 
		for(int i = 0; i < ranks; i++) recv_amount += amounts[i];
		Message *rank_bucket = new Message[recv_amount]; // Gather into this

		for(int i = 1; i < ranks; i++) recv_displacements[i] = recv_displacements[i-1] + amounts[i-1];

		MPI_Alltoallv(send_buckets, local_amounts, send_displacements, message_struct, rank_bucket, amounts, recv_displacements, message_struct, MPI_COMM_WORLD);
		delete[](amounts);
		delete[](local_amounts);
		delete[](send_displacements);
		delete[](send_buckets);

		unordered_map<string,uint64_t> bucket; // All my words
		for(int i = 0; i < recv_amount; i++) bucket[rank_bucket[i].word] += rank_bucket[i].count;

		delete[](rank_bucket);

	/* Other guys send their parts while master gets everything and prints */
		int bucket_size = bucket.size();
		int *bucket_sizes = new int[ranks];
		int *displacements = new int[ranks];

		MPI_Gather(&bucket_size, 1, MPI_INT, bucket_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

		int final_bucket_size = 0;
		for(int i = 0; i < ranks; i++) final_bucket_size += bucket_sizes[i];
		displacements[0] = 0;
		for(int i = 1; i < ranks; i++) displacements[i] = displacements[i-1] + bucket_sizes[i-1];

		Message *final_bucket_array;
		if(rank == 0) final_bucket_array = new Message[final_bucket_size]; // Gather into this

		Message *bucket_array = new Message[bucket_size]; // Gather from this
		int i = 0;
		for(auto &p : bucket) {
			bucket_array[i] = Message(p.second, p.first.c_str());
			i++;
		}

		MPI_Gatherv(bucket_array, bucket_size, message_struct, final_bucket_array, bucket_sizes, displacements, message_struct, 0, MPI_COMM_WORLD);
		delete[](bucket_sizes);
		delete[](displacements);
		delete[](bucket_array);

		if(rank == 0) {
			map<string,uint64_t> final_bucket;
			for(int i = 0; i < final_bucket_size; i++) final_bucket[final_bucket_array[i].word] = final_bucket_array[i].count;
			delete[](final_bucket_array);

			double end_time = MPI_Wtime();

			// If in debug mode, we don't care about the words.
			#ifndef DEBUG
				#ifdef COUNTSORT
					vector<pair<string,int>> wordsToSort;
					for(auto &p : final_bucket) wordsToSort.emplace_back(p.first,p.second);
					sort(wordsToSort.begin(),wordsToSort.end(),[](pair<string,int> &e1, pair<string,int> &e2){return e1.second;});
					for(auto &p : wordsToSort) ofs << "(" << p.first << "," << p.second << ")" << endl;
				#else
					for(auto &p : final_bucket) ofs << "(" << p.first << "," << p.second << ") in 0 (final count[down])" << endl;
				#endif
			#endif
			ofs << "Time excluding printing = " << (end_time-start_time) << endl;
			
			#ifndef DEBUG 
				ofs << "Time including printing = " << (MPI_Wtime()-start_time) << endl;			     
			#endif
		}

	/* Free memory and finalize */
		MPI_File_close(&f);
		if(rank == 0) ofs.close();
		MPI_Finalize();
}
