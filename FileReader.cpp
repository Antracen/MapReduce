#include "FileReader.h"
#include <string>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <unordered_map>

using std::string;
using std::cout;
using std::endl;
using std::stringstream; 
using std::getline;
using std::unordered_map;

void readFile(char* filename,int rank, unordered_map<string,int>& m){
    MPI_File f;
	cout << "I am rank " << rank << endl;
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	// MPI_Offset filesize;
	// MPI_File_get_size(f,&filesize);
	
	// int localsize = filesize/ranks;
	char *alice = (char*) malloc(1000*sizeof(char)); 
	MPI_File_read_at(f,1000*rank,alice,1000,MPI_CHAR, MPI_STATUS_IGNORE);
	//alice[99] = '\0';
	//istream i(alice);
	

	stringstream ss(alice);
	string word;
	//unordered_map<string,int> m;
	while(getline(ss,word,' ')){
		if(m.count(word)) m[word] = m[word]+1; // Local reduce here? 
		else m[word]=1;
	}
	//for(auto it = m.begin(); it!=m.end(); it++) cout << "<" << it->first << "," << it->second << ">";
	cout << "done here" << endl; 
	//return m;
}
