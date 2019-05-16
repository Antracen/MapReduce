CC_beskow = CC
CC_local = mpiCC

FLAGS = -std=c++11 -g -Wall -pedantic

beskow:
	$(CC_beskow) $(FLAGS) -o wordcount.out wordcount.cpp

clean:
	rm -f *.o *.out

local:
	$(CC_local) $(FLAGS) -o wordcount.out wordcount.cpp