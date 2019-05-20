// TODO:
	// Free memory when not needed anymore
	// Make sure it handles running on only one process
	// Make sure it handles running on "too many" processes (some get no chunks)
	// Do we need a more explicit "reduce" call?
	// Can we utilize OpenMP?
	// Can we utilize operations such as gather, alltoall, scatter etc?
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

using std::map;
using std::string;
using std::cout;
using std::endl;

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	double start_time = MPI_Wtime();

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
		int total_chunks = file_size / CHUNK_SIZE;
		int chunks_per_process = total_chunks / ranks;
		int left_chunks = total_chunks % ranks;
		if(left_chunks != 0) {
			if(rank + 1 <= left_chunks) chunks_per_process += 1;
		}		
		int extra_chunk = file_size % CHUNK_SIZE;

		if(rank == 0) {
			cout << "File size: " << file_size << endl;
			cout << "Total chunks: " << total_chunks << endl;
		}

		if(rank != 0) cout << "Rank " << rank << " reads " << chunks_per_process << " chunks " << endl;
		else cout << "Master reads " << chunks_per_process << " chunks and " <<  extra_chunk << " extra data " << endl;

	/* Map the chunks into KV pairs */
		std::vector<std::map<std::string, uint64_t>> buckets(ranks);
		char *buf = (char*) malloc(CHUNK_SIZE + 1);
		char *word = (char*) malloc(CHUNK_SIZE + 1);
		//%#OMP?
		for(int i = 0; i < chunks_per_process; i++) {
			MPI_File_read_at(f, rank*CHUNK_SIZE*chunks_per_process + i*CHUNK_SIZE, buf, CHUNK_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
			buf[CHUNK_SIZE] = '\0';
			int c = 0;
			while(c < CHUNK_SIZE) {
				int w = 0;
				while(!isalnum(buf[c]) && c < CHUNK_SIZE) c++;

				while(c < CHUNK_SIZE && (isalnum(buf[c]) || buf[c] == '\'')) {
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
			cout << "Reading extra big chungus. Time: " << (MPI_Wtime() - start_time) << endl;			
			MPI_File_read_at(f, ranks*CHUNK_SIZE*chunks_per_process, buf, extra_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
			cout << "Chungus read. Time: " << (MPI_Wtime() - start_time) << endl;			
			buf[extra_chunk] = '\0';
			int c = 0;
			while(c < extra_chunk) {
				int w = 0;
				while(!isalnum(buf[c]) && c < extra_chunk) c++;

				while(c < extra_chunk && (isalnum(buf[c]) || buf[c] == '\'')) {
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

		cout << "Map phase done. Rank: " << rank << " time: " << (MPI_Wtime() - start_time) << endl;

	/* Send the data to each owning process */
		// Calculate (1) how much to send to everyone and (2) how much I will receive
		int *receive_amount = new int[ranks]; // How much will I receive in total
		for(int i = 0; i < ranks; i++){ 
			int num_words = buckets[i].size();
			MPI_Allreduce(&num_words, &receive_amount[i], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
		}

		// Send the words to their rightful owner
		MPI_Request *requests = new MPI_Request[ranks];
		MPI_Request *requestsCount = new MPI_Request[ranks];
		for(int i = 0; i < ranks; i++) {
			int count = 0;
			for(auto &p : buckets[i]) {
				const char *word = p.first.c_str();
				MPI_Isend(&p.second,1,MPI_INT,i,count,MPI_COMM_WORLD,&requestsCount[i]); // Really need unique tag here?
				MPI_Isend(word,strlen(word),MPI_CHAR,i,count,MPI_COMM_WORLD,&requests[i]);
				count++;
			}
		}

		cout << "Sent all data " << (MPI_Wtime() - start_time) << endl;
        
		map<string,int> bucket; // All my worlds
		// Receive my words that the other guys had
		int amount = receive_amount[rank]; // How much I should receive
		int sizeOfIncoming, count;
		while(amount > 0) {
			MPI_Status s1,s2;
			MPI_Recv(&count,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s1); // Get a count
			MPI_Probe(s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,&s2); // Find out who sends (who sent the count)
			MPI_Get_count(&s2, MPI_CHAR, &sizeOfIncoming); // Get length of incoming word
			char *word = new char[sizeOfIncoming+1];
			MPI_Recv(word,sizeOfIncoming,MPI_CHAR,s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			word[sizeOfIncoming] = '\0';
			bucket[word] = (bucket.count(word)) ? bucket[word] + count : count;
			amount--; 
        }

		cout << "Received my data " << (MPI_Wtime() - start_time) << endl;

	/* Other guys send their parts while master gets everything and prints */
		int bucket_size = bucket.size();
		MPI_Reduce(&bucket_size, &amount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		amount = amount - bucket_size;
		if(rank != 0) {
			int count = 0; 
			for(auto &p : bucket) {
				const char *word = p.first.c_str();
				MPI_Isend(&p.second,1,MPI_INT,0,count,MPI_COMM_WORLD,requestsCount); // New request-arrays? 
				MPI_Isend(word,strlen(word),MPI_CHAR,0,count,MPI_COMM_WORLD,requests);
				count++;
			}
		} else {		
			// Ta emot skit 
			while(amount > 0) {
				MPI_Status s1,s2;
				MPI_Recv(&count,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s1); // Get a count
				MPI_Probe(s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,&s2); // Find out who sends (who sent the count)
				MPI_Get_count(&s2, MPI_CHAR, &sizeOfIncoming); // Get length of incoming word
				char *word = new char[sizeOfIncoming + 1];
				MPI_Recv(word,sizeOfIncoming,MPI_CHAR,s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				word[sizeOfIncoming] = '\0';
				bucket[word] = (bucket.count(word)) ? bucket[word] + count : count;
				amount--; 
			}

			cout << "Master has all words " << (MPI_Wtime() - start_time) << endl;

			for(auto &p : bucket) {
				cout << "(" << p.first << "," << p.second << ")" << endl;
			}
			double time = MPI_Wtime() - start_time;
			cout << "Time: " << time << endl;
		}

	MPI_Finalize();
}