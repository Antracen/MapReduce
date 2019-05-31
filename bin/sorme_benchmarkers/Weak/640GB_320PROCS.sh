#!/bin/bash

#SBATCH -J FINAL_COUNTDOWN
#SBATCH -t 00:10:00
#SBATCH -A edu19.DD2356
#SBATCH --nodes=10
#SBATCH -e error_file.e
#SBATCH --mail-type=END
#SBATCH --mail-user=sorme,mawass

mkdir -p /cfs/nobackup/s/sorme/MapReduce/Results

file_size=640
procs=$((file_size / 2))

if=/cfs/klemming/scratch/s/sergiorg/DD2356/input/wikipedia_${file_size}GB.txt
of=/cfs/nobackup/s/sorme/MapReduce/Results/size_${file_size}GB_procs_${procs}.txt

export OMP_NUM_THREADS=32
aprun -q -n $procs /cfs/nobackup/s/sorme/MapReduce/bin/wordcount.out $if $of
