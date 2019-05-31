#!/bin/bash

for filename in /cfs/nobackup/m/mawass/MapReduce/bin/Benchmarkers/Strong/*
do
    sbatch ${filename}
done
