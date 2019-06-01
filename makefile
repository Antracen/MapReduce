CC_local=mpiCC
CC_beskow=CC

FLAGS_local = -std=c++11 -g -Wall -pedantic -fopenmp -D WORD_SIZE=300 -D CHUNK_SIZE=6000 -D MAX_CONCURRENT_CHUNKS=10
FLAGS_beskow = -std=c++11 -g -Wall -pedantic -fopenmp -D WORD_SIZE=300 -D CHUNK_SIZE=67108864 -D MAX_CONCURRENT_CHUNKS=15

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
	$(CC_beskow) $(FLAGS_beskow) -D DEBUG -o bin/debug_wordcount.out bin/iohandler.o src/wordcount.cpp

profile_beskow: iohandler_beskow
	$(CC_beskow) $(FLAGS_beskow) -lmap-sampler -o bin/wordcount.out bin/iohandler.o src/wordcount.cpp

iohandler_beskow: folder
	$(CC_beskow) $(FLAGS_beskow) -o bin/iohandler.o -c src/iohandler.cpp
