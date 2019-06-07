#!/bin/bash

for filename in /cfs/nobackup/s/sorme/MapReduce/bin/Benchmarkers/Strong/*
do
    sbatch ${filename}
done
