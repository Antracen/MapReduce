#include "commhandler.h"
#include <iostream>
#include <map>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

#include <string>

using std::map; 
using std::cout; 
using std::endl; 
using std::string;

void receive_words(map<string, uint64_t> &bucket, int amount) {
    while(amount > 0) {			
        int count;
        int sizeOfIncoming; 
        MPI_Status s1,s2;
        MPI_Recv(&count,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s1); // Get a count
        MPI_Probe(s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,&s2); // Find out who sends (who sent the count)
        MPI_Get_count(&s2, MPI_CHAR, &sizeOfIncoming); // Get length of incoming word
        char *word = new char[sizeOfIncoming];
        MPI_Recv(word,sizeOfIncoming,MPI_CHAR,s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       	#ifdef DEBUG
		cout << "("<<word << "," << count << ") received by " << 0 << endl;
		#endif
        bucket[word] = (bucket.count(word)) ? bucket[word] + count : count;
        amount--; 
    }
}

void send_words(map<string, uint64_t> &bucket, int receiver){
    MPI_Request r1, r2;
    int countTag = 0; 
    for(auto &p : bucket) {
		const char *word = p.first.c_str();
		MPI_Isend(&p.second,1,MPI_INT,receiver,countTag,MPI_COMM_WORLD,&r1); // Really need unique tag here?
		MPI_Isend(word,strlen(word)+1,MPI_CHAR,receiver,countTag,MPI_COMM_WORLD,&r2);
		#ifdef DEBUG
		cout << "("<<word << "," << p.second << ") sent to " << receiver << endl; 
		#endif
        countTag++; 
	}
}
