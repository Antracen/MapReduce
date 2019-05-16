#include <iostream>
#include <string>
#include <mpi.h>

using std::cout;
using std::endl;
using std::string;
using std::istream;

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);
	int rank, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&ranks); 

	MPI_File f;
	if(argc < 2) return -1;
	char* filename = argv[1];	
	cout << rank << endl;
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	MPI_Offset filesize;
	MPI_File_get_size(f,&filesize);
	
	int localsize = filesize/ranks;
	char *alice = (char*) malloc(100*sizeof(char)); 
	MPI_File_read(f,alice,100,MPI_CHAR,MPI_STATUS_IGNORE);
	alice[99] = '\0';
	
	cout << alice << endl;

	MPI_Finalize(); 


}
