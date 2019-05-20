CC_local = mpiCC

FLAGS = -std=c++11 -g -Wall -pedantic -fopenmp

clean:
	rm -f *.o *.out

local:
	$(CC_local) $(FLAGS) -o wordcount.out wordcount.cpp