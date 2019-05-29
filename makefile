CC_local=mpiCC
CC_beskow=CC

FLAGS_local = -std=c++11 -g -Wall -pedantic -fopenmp -D WORD_SIZE=300 -D CHUNK_SIZE=64000 -D MAX_CONCURRENT_CHUNKS=10
FLAGS_beskow = -std=c++11 -g -Wall -pedantic -fopenmp -D WORD_SIZE=300 -D CHUNK_SIZE=64000000 -D MAX_CONCURRENT_CHUNKS=10

clean:
	rm -f *.o *.out

folder:
	mkdir -p bin

# LOCAL PART
local: iohandler_local
	$(CC_local) $(FLAGS_local) -o bin/wordcount.out bin/iohandler.o src/wordcount.cpp

debug_local: iohandler_local
	$(CC_local) $(FLAGS_local) -D DEBUG -o bin/debug_wordcount.out bin/iohandler.o src/wordcount.cpp

iohandler_local: folder
	$(CC_local) $(FLAGS_local) -o bin/iohandler.o -c src/iohandler.cpp

# BESKOW PART
beskow: iohandler_beskow
	$(CC_beskow) $(FLAGS_beskow) -o bin/wordcount.out bin/iohandler.o src/wordcount.cpp

debug_beskow: iohandler_beskow
	$(CC_beskow) $(FLAGS_beskow) -D DEBUG -o bin/debug_wordcount.out src/wordcount.cpp

profile_beskow: folder
	$(CC_beskow) $(FLAGS_beskow) -lmap-sampler --eh-frame-hdr -o bin/wordcount.out src/wordcount.cpp

iohandler_beskow: folder
	$(CC_beskow) $(FLAGS_beskow) -o bin/iohandler.o -c src/iohandler.cpp
