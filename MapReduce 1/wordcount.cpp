#define TOO_FEW_ARGUMENTS 007
#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "FileReader.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>

using std::string;
using std::istream;
using std::pair;
using std::vector;
using std::unordered_map;
using namespace std;

int main(int argc, char *argv[]){
	srand(time(NULL));
	MPI_Init(&argc,&argv);

	if(argc < 2) MPI_Abort(MPI_COMM_WORLD, TOO_FEW_ARGUMENTS);

	int rank, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&ranks);
	
	vector<unordered_map<string,int>> buckets(ranks);
	readFile(argv[1],rank,ranks,buckets);

	int *amountToSend = new int[ranks]; 
	for(int i = 0; i < ranks; i++){ 
		int wordsToThisGuy = buckets[i].size(); 
		MPI_Allreduce(&wordsToThisGuy,&amountToSend[i],1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
	}
	for(int i = 0; i < ranks; i++) cout << "Bucket (total): " << amountToSend[i] << "," <<  endl;
	

	
	// Send the words to their rightful owner
	MPI_Request *requests = new MPI_Request[ranks];
	for(size_t i = 0; i < buckets.size(); i++) {
		for(auto &p : buckets[i]){
			const char *word = p.first.c_str();
			//int data[] = {p.second,p.first.length()}; // First send count and word length
			char *data = new char[sizeof(int)+strlen(word)];

			data = (char*)&p.second; // The count
			//data[0] = 1;
			//data[1] = 1;
			//data[2] = 1;
			//data[3] = 1;
			cout << p.first << ": Count in data: " << data[0]+data[1]+data[2]+data[3] << ". real: " << p.second << endl; 
			for(size_t j = 0; j < strlen(word); j++) data[4+j] = word[j];
			cout << "Whole data: " << data << endl; 
			MPI_Isend(word,strlen(word),MPI_CHAR,i,rand()%1000,MPI_COMM_WORLD,requests);
		}
	}
	
	unordered_map<string,int> bucket; 
	// NOTE: We will also receive words from our selves 

	// Receive my words that the other guys had
	int amount = amountToSend[rank]; // How much I should receive
	while(amount > 0){
		MPI_Status status; 
		int sizeOfIncoming; 
		MPI_Probe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		MPI_Get_count(&status, MPI_CHAR, &sizeOfIncoming);
		char *message = new char[sizeOfIncoming];
		MPI_Recv(message,sizeOfIncoming,MPI_CHAR,status.MPI_SOURCE,status.MPI_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//int count = message[0]+message[1]+message[2]+message[3]; 
		cout << "Message " << message <<  endl; 
		//char word[counts[1]]; // Here we use the said length of upcoming word
		//MPI_Recv(word,counts[1],MPI_CHAR,i,1337,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//buckets[rank][word] = (buckets[rank].count(word)) ? buckets[rank][word]+1 :  1;
		amount--; 
	}
	

	//for(auto& word : buckets[rank]) cout << "("<<word.first << "," << word.second << ")" << endl;
	
	MPI_Finalize(); 
}
