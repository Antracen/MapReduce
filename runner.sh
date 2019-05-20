#!/bin/bash

#SBATCH -J MapReduce
#SBATCH -t 00:40:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=5
#SBATCH -e error_file.e
#SBATCH --mail-type=END
#SBATCH --mail-user=sorme,mawass,tedwards

mkdir -p Results

aprun -q -n 80 ./wordcount.out /cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_10GB.txt >> Results/10GB_80.csv
aprun -q -n 160 ./wordcount.out /cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_10GB.txt >> Results/10GB_160.csv

