#!/bin/bash

#SBATCH -J MapReduce
#SBATCH -t 00:10:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=5
#SBATCH -e error_file.e
#SBATCH --mail-type=BEGIN,END
#SBATCH --mail-user=sorme,mawass
#SBATCH --output=/cfs/nobackup/m/mawass/MapReduce/Results/runner.txt

mkdir -p Results

export OMP_NUM_THREADS=16
aprun -q -n 160 /cfs/nobackup/m/mawass/MapReduce/bin/wordcount.out /cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_160GB.txt
