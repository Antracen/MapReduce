#include "FileReader.h"
#include <string>
#include <iostream>
#include <mpi.h>

using std::string;
using std::cout;
using std::endl;
using std::istream; 

void readFile(char* filename,int rank){
    MPI_File f;
	cout << "skrr" << endl;
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	// MPI_Offset filesize;
	// MPI_File_get_size(f,&filesize);
	
	// int localsize = filesize/ranks;
	char *alice = (char*) malloc(100*sizeof(char)); 
	MPI_File_read(f,alice,100,MPI_CHAR, MPI_STATUS_IGNORE);
	//alice[99] = '\0';
	istream i(alice);
	
	while()
	cout << alice << endl;
}