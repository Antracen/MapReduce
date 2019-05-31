#!/bin/bash

for filename in /cfs/nobackup/m/mawass/MapReduce/bin/Benchmarkers/Weak/*
do
    sbatch ${filename}
done
