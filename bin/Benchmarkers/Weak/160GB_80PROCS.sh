#!/bin/bash

#SBATCH -J FINAL_COUNTDOWN
#SBATCH -t 00:15:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=5
#SBATCH -e error_file.e
#SBATCH --mail-type=END
#SBATCH --mail-user=sorme,mawass

mkdir -p /cfs/nobackup/m/mawass/MapReduce/Results

file_size=160
procs=$file_size

if=/cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_${file_size}GB.txt
of=/cfs/nobackup/m/mawass/MapReduce/Results/size_${file_size}GB_procs_${procs}.txt

echo "" > $of

export OMP_NUM_THREADS=32
aprun -q -n $procs /cfs/nobackup/m/mawass/MapReduce/bin/wordcount.out $if $of
