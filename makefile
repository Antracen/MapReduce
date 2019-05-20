CC_local=mpiCC
CC_beskow=CC

FLAGS = -std=c++11 -g -Wall -pedantic -fopenmp

clean:
	rm -f *.o *.out

local:
	$(CC_local) $(FLAGS) -o wordcount.out wordcount.cpp

beskow:
	$(CC_beskow) $(FLAGS) -o wordcount.out wordcount.cpp

profile_beskow:
	$(CC_beskow) $(FLAGS) -lmap-sampler --eh-frame-hdr -o wordcount.out wordcount.cpp
