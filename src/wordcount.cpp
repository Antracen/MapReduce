// TODO
	// What about local reduction when having all_buckets? Merge to one instead? Atomic?
    // Free memory when not needed anymore
    // Do we need a more explicit "reduce" call?
    // Can we utilize OpenMP further?
    // Can we utilize operations such as gather, alltoall, scatter etc?
    // Which variables should be unsigned long int?
    // Can we use Ireduce?
    // Can we utilize padding?
	// Should we have an unordered map?
	// Make sure we free the memory allocated in Isend in the commhandler
#define TOO_FEW_ARGUMENTS 007
#define NONEXISTENT_FILE 1919

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
using std::max;

int main(int argc, char *argv[]){
    /* Initialize */
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

	/* Create Message struct */
		MPI_Datatype message_struct;
		MPI_Type_create_struct(2, new int[2]{1,WORD_SIZE},new MPI_Aint[2]{0,sizeof(uint64_t)}, new MPI_Datatype[2]{MPI_UNSIGNED_LONG,MPI_CHAR}, &message_struct);
		MPI_Type_commit(&message_struct);

    /* Calculate what to read */
        MPI_Offset file_size;
        MPI_File_get_size(f, &file_size);

        int num_chunks_total = file_size / CHUNK_SIZE;
        int extra_chunk = file_size % CHUNK_SIZE;
        int num_chunks_local = num_chunks_total / ranks;
        if(num_chunks_total % ranks > rank) num_chunks_local++;

    /* Map the chunks into KV pairs */
        vector<map<string, unsigned long int>> buckets(ranks);

        size_t buf_size = max((CHUNK_SIZE+1)*num_chunks_local, extra_chunk + 1);
        char* buf = (char*) malloc(buf_size);
        char* word = (char*) malloc(buf_size);

        #pragma omp parallel for
        for(int i = 0; i < num_chunks_local; i++) {
            int chunk = rank + i*ranks;
            MPI_File_read_at(f, chunk*CHUNK_SIZE, &buf[i*(CHUNK_SIZE+1)], CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
            buf[i*(CHUNK_SIZE+1) + CHUNK_SIZE] = '\0';
            read_chunk(word, &buf[i*(CHUNK_SIZE+1)], CHUNK_SIZE, buckets, ranks);
        }
        #ifdef DEBUG
            cout << "Regular chunks done reading. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

        if(extra_chunk != 0 && rank == 0) {
            MPI_File_read_at(f, file_size - extra_chunk, buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
            buf[extra_chunk] = '\0';
            read_chunk(word, buf, extra_chunk, buckets, ranks);
        }
        #ifdef DEBUG
            if(rank == 0) cout << "Extra chunk done reading. (master, time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

		free(buf);
		free(word);

        #ifdef DEBUG
            cout << "Map phase done. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

    /* Send the data to each owning process */
        // Calculate (1) how much to send to everyone and (2) how much I will receive
        //int *receive_amount = new int[ranks]; // How much will I receive in total
        int amount;

        for(int i = 0; i < ranks; i++) {
            int num_words = buckets[i].size();
            MPI_Reduce(&num_words,&amount,1,MPI_INT,MPI_SUM,i,MPI_COMM_WORLD);
        }

        // Send the words to their rightful owner
        for(int i = 0; i < ranks; i++) send_words(buckets[i],i,message_struct);

        #ifdef DEBUG
            cout << "Sent words. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

        map<string,unsigned long int> bucket; // All my words
        // Receive my words that the other guys had

        receive_words(bucket, amount, message_struct);
        #ifdef DEBUG
            cout << "Received all words. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

    /* Other guys send their parts while master gets everything and prints */
        int bucket_size = bucket.size();
		int *bucket_sizes = new int[ranks];
		int *displacements = new int[ranks];
		#ifdef DEBUG
			cout << "ALLOCATED ARRAYS" << endl;
		#endif
		MPI_Gather(&bucket_size, 1, MPI_INT, bucket_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);
		int final_bucket_size = 0;
		for(int i = 0; i < ranks; i++) final_bucket_size += bucket_sizes[i];
		displacements[0] = 0;
		for(int i = 1; i < ranks; i++) displacements[i] = displacements[i-1] + bucket_sizes[i-1];

		Message *final_bucket_vector;
		if(rank == 0) final_bucket_vector = new Message[final_bucket_size]; // Gather into this

		Message *bucket_vector = new Message[bucket_size]; // Gather from this
		 #ifdef DEBUG
            cout << "ALLOCATED ARRAYS(2)" << endl;
        #endif
		

		int i = 0;
		for(auto &p : bucket) {
			bucket_vector[i] = Message(p.second, p.first.c_str());
			i++;
		}

		MPI_Gatherv(bucket_vector, bucket_size, message_struct, final_bucket_vector, bucket_sizes, displacements, message_struct, 0, MPI_COMM_WORLD);

		if(rank == 0) {
			map<string,unsigned long int> final_bucket;
			for(int i = 0; i < final_bucket_size; i++) final_bucket[final_bucket_vector[i].word] = final_bucket_vector[i].count;

		    double end_time = MPI_Wtime();

		    // If in debug mode, we don't care about the words.
        	    #ifndef DEBUG
	                #ifdef COUNTSORT
	                    vector<pair<string,int>> wordsToSort;
	                    for(auto &p : final_bucket) wordsToSort.emplace_back(p.first,p.second);
	                    sort(wordsToSort.begin(),wordsToSort.end(),[](pair<string,int> &e1, pair<string,int> &e2){return e1.second);
	                    for(auto &p : wordsToSort) cout << "(" << p.first << "," << p.second << ")" << endl;
	                    double time = MPI_Wtime() - start_time;
	                    cout << "Time: " << time << endl;
	                #else
	                    for(auto &p : final_bucket) cout << "(" << p.first << "," << p.second << ") in 0 (final count[down])" << endl;
	                    double time = MPI_Wtime() - start_time;
	                    cout << "Time: " << time << endl;
	                #endif
	            #endif
		    cout << "Time excluding printing = " << (end_time-start_time) << endl;
		    #ifndef DEBUG											      	
		    	cout << "Time including printing = " << (MPI_Wtime()-start_time) << endl;			     
		    #endif												     
        }

    /* Free memory and finalize */
        MPI_Finalize();
}

