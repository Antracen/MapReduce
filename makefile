CC = CC

FLAGS = -std=c++11 -g -Wall -pedantic

all:
	$(CC) $(FLAGS) -o wordcount.out wordcount.cpp

clean:
	rm -f *.o *.out
