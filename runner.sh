#!/bin/bash

#SBATCH -J MapReduce
#SBATCH -t 00:40:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=5
#SBATCH -e error_file.e
#SBATCH --mail-type=END
#SBATCH --mail-user=sorme,mawass

mkdir -p Results

aprun -q -n 160 ./wordcount.out /cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_160GB.txt >> Results/160GB_160.csv

