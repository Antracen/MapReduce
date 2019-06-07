#!/bin/bash

for filename in /cfs/nobackup/s/sorme/MapReduce/bin/Benchmarkers/Weak/*
do
    sbatch ${filename}
done
