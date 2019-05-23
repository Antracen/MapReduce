#!/bin/bash

#SBATCH -J MapReduce
#SBATCH -t 00:15:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=5
#SBATCH -e error_file.e
#SBATCH --mail-type=BEGIN,END
#SBATCH --mail-user=sorme,mawass

mkdir -p Results

echo "Hey baby" > Results/160GB_160.csv

aprun -q -n 160 ./wordcount.out /cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_160GB.txt >> Results/160GB_160.csv
