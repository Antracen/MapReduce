#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using std::cout;
using std::endl;
using std::string;
using std::istream;
using std::pair;
using std::vector;
using std::unordered_map;


int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	int rank, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&ranks); 

	// MPI_File f;
	// if(argc < 2) return -1;
	// char* filename = argv[1];	
	cout << rank << endl;
	// MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	// MPI_Offset filesize;
	// MPI_File_get_size(f,&filesize);
	
	// int localsize = filesize/ranks;
	// char *alice = (char*) malloc(100*sizeof(char)); 
	// MPI_File_read(f,alice,100,MPI_CHAR,MPI_STATUS_IGNORE);
	// alice[99] = '\0';
	
	// cout << alice << endl;
	
	unordered_map<string,int> bucket;

	vector<pair<string,int>> v;
	for(int i = 0; i < 100; i++) {
		bucket.insert(pair<string,int> {"abc" + std::to_string(i), i*i});
	}

	// Hashing 
	std::hash<string> hasher;
	MPI_Request requests[ranks];
	for(auto &p : bucket) {
		int h = hasher(p.first) % ranks;
		int length = p.first.length();
		const char *word = p.first.c_str();
		MPI_Send(&length,1,MPI_INT,h,1337,MPI_COMM_WORLD); 
		MPI_Send(word,p.first.length(),MPI_CHAR,h,1337,MPI_COMM_WORLD); 
		break;
	}


	MPI_Finalize(); 
}
