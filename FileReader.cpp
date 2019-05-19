#include "FileReader.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <mpi.h>
#include <sstream>
#include <unordered_map>
#include <regex>

using std::string;
using std::cout;
using std::endl;
using std::stringstream; 
using std::getline;
using std::unordered_map;
using std::regex;
using std::regex_replace;

void readFile(char* filename,int rank, unordered_map<string,int>& m){
    MPI_File f;
	cout << "I am rank " << rank << endl;
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	// MPI_Offset filesize;
	// MPI_File_get_size(f,&filesize);
	
	// int localsize = filesize/ranks;
	char *alice = (char*) malloc(1000*sizeof(char)); 
	MPI_File_read_at(f,1000*rank,alice,1000,MPI_CHAR, MPI_STATUS_IGNORE);
	//alice[999] = '\0';
	regex r("\\W+");
	string chapter = regex_replace(alice,r," "); // Replace crap with " "

	stringstream ss(chapter);
	string word;
	while(getline(ss,word,' ')){
		transform(word.begin(), word.end(), word.begin(), tolower);

		if(m.count(word)) m[word] = m[word]+1; // Local reduce here? 
		else m[word]=1;
	}
}
