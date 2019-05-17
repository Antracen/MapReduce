#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "FileReader.h"


using std::string;
using std::istream;
using std::pair;
using std::vector;
using std::unordered_map;
using namespace std;

int main(int argc, char *argv[]){
	MPI_Init(&argc,&argv);

	if(argc < 2) MPI_Abort(MPI_COMM_WORLD, 007);


	int rank, ranks;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&ranks); 
	cout << "Greetings from "<< rank << "in main" << endl;
	MPI_Finalize();return 0;
	readFile(argv[1], rank);

	
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
