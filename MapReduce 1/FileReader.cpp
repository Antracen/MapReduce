#include "FileReader.h"

void readFile(char* filename,int rank, int ranks, vector<unordered_map<string,int>>& buckets){
    MPI_File f;
	MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
	
	// MPI_Offset filesize;
	// MPI_File_get_size(f,&filesize);
	
	// int localsize = filesize/ranks;
	char *alice = (char*) malloc(100*sizeof(char)); 
	MPI_File_read_at(f,100*rank,alice,100,MPI_CHAR, MPI_STATUS_IGNORE);
	alice[99] = '\0';
	regex r("\\W+");
	string chapter = regex_replace(alice,r," "); // Replace crap with " "

	stringstream ss(chapter);
	string word;
	while(getline(ss,word,' ')){
		transform(word.begin(), word.end(), word.begin(), tolower);
		std::hash<string> hasher;
		int h = hasher(word) % ranks;
		buckets[h][word] = (buckets[h].count(word)) ? buckets[h][word]+1 : 1; // Local reduce here? 
	}
}
