CC_beskow = CC
CC_local = mpiCC

FLAGS = -std=c++11 -g -Wall -pedantic

beskow: FileReader_beskow
	$(CC_beskow) $(FLAGS) -o wordcount.out wordcount.cpp

clean:
	rm -f *.o *.out

FileReader_beskow:
	$(CC) $(FLAGS) -c FileReader.cpp

FileReader_local:
	$(CC_local) $(FLAGS) -c FileReader.cpp

local: FileReader_local
	$(CC_local) $(FLAGS) -o wordcount.out FileReader.o wordcount.cpp

run_local: local
	# cpulimit -l 1 -- mpirun  -n 3 ./wordcount.out alice.txt
	mpirun  -n 3 ./wordcount.out alice.txt
