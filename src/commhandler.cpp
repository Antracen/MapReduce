#include "commhandler.h"
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;

using std::map; 
using std::cout; 
using std::endl; 
using std::string;
using std::vector;

void receive_words(map<string, uint64_t> &bucket, int amount) {
    //while(amount > 0) {

	typedef map<string,uint64_t> buck;
	#pragma omp declare reduction(+ : buck : transform(omp_out.begin(),omp_out.end(),omp_in.begin(),omp_out.begin(), [](uint64_t &c1,uint64_t &c2){c1 += c2;}  )) initializer (omp_priv(omp_out)) 


    #pragma omp parallel for reduction (+:bucket)
    for(int i = amount; i > 0; i--){
        int count;
        int sizeOfIncoming; 
        MPI_Status s1,s2;
        MPI_Recv(&count,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s1); // Get a count
        MPI_Probe(s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,&s2); // Find out who sends word associated with count (who sent the count)
        MPI_Get_count(&s2, MPI_CHAR, &sizeOfIncoming); // Get length of incoming word
        char *word = new char[sizeOfIncoming];
        MPI_Recv(word,sizeOfIncoming,MPI_CHAR,s1.MPI_SOURCE,s1.MPI_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
       	#ifdef DEBUG
		    cout << "("<<word << "," << count << ") received by " << 0 << endl;
		#endif
	//#pragma omp critical
       	bucket[word] += count;
	//amount--; 
    }
}

void send_words(map<string, uint64_t> &bucket, int receiver){
    MPI_Request r1, r2;
    int countTag = 0; 
    for(auto &p : bucket) {
		const char *word = p.first.c_str();
		MPI_Isend(&p.second,1,MPI_INT,receiver,countTag,MPI_COMM_WORLD,&r1); // Really need unique tag here?
		MPI_Isend(word,strlen(word)+1,MPI_CHAR,receiver,countTag,MPI_COMM_WORLD,&r2);
		#ifdef DEBUG
		cout << "("<<word << "," << p.second << ") sent to " << receiver << endl; 
		#endif
        countTag++; 
	}
}
