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

#define DEBUG
// #define COUNTSORT

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

    /* Map the chunks into KV pairs */
        int all_buckets_size = (extra_chunk == 0) ? chunks_to_read.size() : chunks_to_read.size() + 1;
        vector<vector<map<string, uint64_t>>> all_buckets(all_buckets_size);
        size_t buf_size = max((CHUNK_SIZE+1)*chunks_to_read.size(), extra_chunk+1);
        char* buf = (char*) malloc(buf_size);
        char* word = (char*) malloc(buf_size);

        #pragma omp parallel for
        for(size_t i = 0; i < chunks_to_read.size(); i++) {
            int chunk = chunks_to_read[i];
            vector<map<string, uint64_t>> buckets(ranks);
            MPI_File_read_at(f, chunk*CHUNK_SIZE, &buf[i*(CHUNK_SIZE+1)], CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
            buf[i*(CHUNK_SIZE+1) + CHUNK_SIZE] = '\0';
            uint64_t c = 0; // DO NOT REMOVE c
            while(c < CHUNK_SIZE) read_word(word, &buf[i*(CHUNK_SIZE+1)], c, CHUNK_SIZE, all_buckets[i], ranks);
        }

        if(extra_chunk != 0 && rank == 0) {
            MPI_File_read_at(f, file_size - extra_chunk, &buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
            buf[extra_chunk] = '\0';
            uint64_t c = 0; // DO NOT REMOVE c
            while(c < extra_chunk) read_word(word, buf, c, extra_chunk, all_buckets[chunks_to_read.size()], ranks);
        }

        #ifdef DEBUG
            cout << "Map phase done. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

    /* Send the data to each owning process */
        // Calculate (1) how much to send to everyone and (2) how much I will receive
        //int *receive_amount = new int[ranks]; // How much will I receive in total
        int amount;

        for(int i = 0; i < ranks; i++) {
            int num_words = 0;
            for(size_t j = 0; j < all_buckets.size(); j++) num_words += all_buckets[j][i].size();
            MPI_Reduce(&num_words,&amount,1,MPI_INT,MPI_SUM,i,MPI_COMM_WORLD);
        }

        // Send the words to their rightful owner
        for(size_t j = 0; j < all_buckets.size(); j++) {
            for(int i = 0; i < ranks; i++) send_words(all_buckets[j][i],i);
        }

        map<string,uint64_t> bucket; // All my worlds
        // Receive my words that the other guys had

        receive_words(bucket, amount);
        #ifdef DEBUG
            cout << "Received all words. (rank: " << rank << ", time: " << (MPI_Wtime() - start_time) << ")" << endl;
        #endif

    /* Other guys send their parts while master gets everything and prints */
        int bucket_size = bucket.size();
        MPI_Reduce(&bucket_size, &amount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        amount = amount - bucket_size;
        if(rank != 0) {
            send_words(bucket,0);
        } else {
            // Receive words
            receive_words(bucket, amount);

            #ifdef DEBUG
                cout << "MapReduce finished, printing results. (time " << (MPI_Wtime() - start_time) << ")" << endl;
            #endif
            #ifdef COUNTSORT
                vector<pair<string,int>> wordsToSort;
                for(auto &p : bucket) wordsToSort.emplace_back(p.first,p.second);
                sort(wordsToSort.begin(),wordsToSort.end(),[](pair<string,int> &e1, pair<string,int> &e2){return e1.second);
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
		free(buf);
		free(word);
        MPI_Finalize();
}
