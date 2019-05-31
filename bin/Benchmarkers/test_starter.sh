#!/bin/bash

weak=true
strong=true

if [ "$weak" = true ] ; then
    for filename in /cfs/nobackup/m/mawass/MapReduce/bin/Benchmarkers/Weak/*
    do
        sbatch ./${filename}
    done
fi

if [ "$strong" = true ] ; then
    for filename in /cfs/nobackup/m/mawass/MapReduce/bin/Benchmarkers/Strong/*
    do
        sbatch ./${filename}
    done
fi