CC_local=mpiCC
CC_beskow=CC

FLAGS = -std=c++11 -g -Wall -pedantic -fopenmp

clean:
	rm -f *.o *.out

folder:
	mkdir -p bin

local: iohandler_local commhandler_local
	$(CC_local) $(FLAGS) -o bin/wordcount.out bin/commhandler.o bin/iohandler.o src/wordcount.cpp

commhandler_local: folder
	$(CC_local) $(FLAGS) -o bin/commhandler.o -c src/commhandler.cpp

iohandler_local: folder
	$(CC_local) $(FLAGS) -o bin/iohandler.o -c src/iohandler.cpp

beskow: iohandler_beskow commhandler_beskow
	$(CC_beskow) $(FLAGS) -o bin/wordcount.out bin/commhandler.o bin/iohandler.o src/wordcount.cpp

commhandler_beskow: folder
	$(CC_beskow) $(FLAGS) -o bin/commhandler.o -c src/commhandler.cpp

iohandler_beskow: folder
	$(CC_beskow) $(FLAGS) -o bin/iohandler.o -c src/iohandler.cpp

profile_beskow: folder
	$(CC_beskow) $(FLAGS) -lmap-sampler --eh-frame-hdr -o bin/wordcount.out src/wordcount.cpp
