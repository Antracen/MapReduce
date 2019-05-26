#pragma once
#include <iostream>
#include <map>
#include <mpi.h>
#include <string>

using std::map; 
using std::cout; 
using std::endl; 
using std::string;

struct Message {
	unsigned long int count;
	char word[WORD_SIZE];
};

void receive_words(map<string, unsigned long int> &bucket, int amount, MPI_Datatype &message_struct);
void send_words(map<string, unsigned long int> &bucket, int receiver, MPI_Datatype &message_struct);

