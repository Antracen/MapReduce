CC_local=mpiCC
CC_beskow=CC

FLAGS = -std=c++11 -g -Wall -pedantic -fopenmp

clean:
	rm -f *.o *.out

local: iohandler_local commhandler_local
	$(CC_local) $(FLAGS) -o bin/wordcount.out bin/commhandler.o bin/iohandler.o src/wordcount.cpp

commhandler_local:
	$(CC_local) $(FLAGS) -c src/commhandler.cpp

iohandler_local:
	$(CC_local) $(FLAGS) -c src/iohandler.cpp

beskow: iohandler_beskow commhandler_beskow
	$(CC_beskow) $(FLAGS) -o bin/wordcount.out bin/commhandler.o bin/iohandler.o src/wordcount.cpp

commhandler_beskow:
	$(CC_beskow) $(FLAGS) -c src/commhandler.cpp

iohandler_beskow:
	$(CC_beskow) $(FLAGS) -c src/iohandler.cpp

profile_beskow:
	$(CC_beskow) $(FLAGS) -lmap-sampler --eh-frame-hdr -o bin/wordcount.out src/wordcount.cpp
