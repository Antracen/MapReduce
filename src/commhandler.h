#pragma once
#include <iostream>
#include <map>
#include <mpi.h>
#include <string>
#include <cstring>

using std::map; 
using std::cout; 
using std::endl; 
using std::string;

class Message {
		public:

	unsigned long int count;
	char word[WORD_SIZE];

	Message(unsigned long int& c,const char *w): count(c) { strcpy(word,w); }
	Message(){}
};

void receive_words(map<string, unsigned long int> &bucket, int amount, MPI_Datatype &message_struct);
void send_words(map<string, unsigned long int> &bucket, int receiver, MPI_Datatype &message_struct);

