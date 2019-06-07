# MapReduce

## Running

The makefile provided in our repository is used both to compile the code and to change the chunk size and maximum word length.

To compile on Beskow run `make beskow` in the repository root. This will compile all files to the `bin` directory. The program executable is called `wordcount.out`. The program expects two arguments, the first being input file and the second output file. See example usage below. 


`aprun -n 160 ./bin/wordcount.out input.txt output.txt`

To run the program locally you compile by running `make local` and then running the executable as explained below.

`mpirun -n 160 ./bin/wordcount.out input.txt output.txt`

In addition to regular compiling locally and on Beskow, the makefile has commands for compiling a debug-version of the program as well as a profiling version for the ARM Map program.

## Result format

Our output files begin by listing all words and the number of times they occurr in the text in ascending alphabetical order. At the end of the output file the execution time including and excluding printing is presented.