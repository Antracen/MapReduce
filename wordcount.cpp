#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "FileReader.h"


using std::string;
using std::istream;
using std::pair;
using std::vector;
using std::unordered_map;
using namespace std;

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	if(argc < 2) MPI_Abort(MPI_COMM_WORLD, 007);


	int rank, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&ranks); 

	unordered_map<string,int> bucket;
	readFile(argv[1],rank,bucket);
	//return 0;
	cout << bucket.size() << endl;
	/*vector<pair<string,int>> v;
	for(int i = 0; i < 100; i++) {
		bucket.insert(pair<string,int> {"abc" + std::to_string(i), i*i});
	}*/

	// Hashing 
	std::hash<string> hasher;
	MPI_Request requestsCount[ranks];
	MPI_Request requestsWord[ranks];

	// Send the words to their rightful owner
	for(auto &p : bucket) {
		int h = hasher(p.first) % ranks;
		const char *word = p.first.c_str();
		int data[] = {p.second,p.first.length()}; // First send count and word length
		MPI_Send(data,2,MPI_INT,h,1337,MPI_COMM_WORLD); //,requestsCount);
		MPI_Send(word,p.first.length(),MPI_CHAR,h,1337,MPI_COMM_WORLD); //,requestsWord);
	}
	cout << "Done sending" << endl;
	// Message the guys that done sending
	for(int i = 0; i < ranks; i++){
		//if(i==rank) continue;
		int doneSendingFromMe = 0;
		MPI_Send(&doneSendingFromMe,1,MPI_INT,i,1337,MPI_COMM_WORLD);
	}
	bucket.clear(); // This is now done, we will receive the true and rightful words to put here later
	// NOTE: We will also receive words from our selves 


	// Receive my words that the other guys had
	int rankStatuses[ranks];
	for(int i = 0; i < ranks; i++){
		//if(rank == i) continue;
		while(1){
			int counts[2];
			MPI_Recv(counts,2,MPI_INT,i,1337,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if(!counts[0]) break; // Sender sent non-positive count, "end of message"
			char word[counts[1]]; // Here we use the said length of upcoming word
			MPI_Recv(word,counts[1],MPI_INT,i,1337,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if(bucket.count(word)) bucket[word] = bucket[word]+1;
			else bucket[word] = 1;
		}
	}

	for(auto& word : bucket) cout << "("<<word.first << "," << word.second << ")" << endl;

	MPI_Finalize(); 
}
