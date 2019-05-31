#!/bin/bash

#SBATCH -J STRONG_COUNTDOWN
#SBATCH -t 00:15:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=3
#SBATCH -e error_file.e
#SBATCH --mail-type=END
#SBATCH --mail-user=sorme,mawass

mkdir -p /cfs/nobackup/m/mawass/Results

file_size=640
procs=80

if=/cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_${file_size}GB.txt
of=/cfs/nobackup/m/mawass/Results/size_${file_size}GB_procs_${procs}_strong.txt

export OMP_NUM_THREADS=32
aprun -q -n $procs /cfs/nobackup/m/mawass/MapReduce/bin/wordcount.out $if $of