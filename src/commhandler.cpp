#include "commhandler.h"
#include <cstring>
#define MESSAGE_TAG 95959

using std::map; 
using std::cout; 
using std::endl; 
using std::string;

void receive_words(map<string, unsigned long int> &bucket, int amount, MPI_Datatype &message_struct) {

	Message message;

    while(amount > 0) {			
        MPI_Recv(&message, 1, message_struct, MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE); // Get a count
		unsigned long int count = message.count;
		string word(message.word);
        bucket[word] = (bucket.count(word)) ? bucket[word] + count : count;
        amount--; 
    }
}

void send_words(map<string, unsigned long int> &bucket, int receiver, MPI_Datatype &message_struct) {
    MPI_Request r1;
	
    for(auto &p : bucket) { 
		Message *message = new Message;
		strcpy(message->word, p.first.c_str());
		message->count = p.second;
		MPI_Isend(message, 1, message_struct,receiver,MESSAGE_TAG,MPI_COMM_WORLD,&r1);
	}
}

