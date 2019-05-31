#!/bin/bash

weak=true
strong=true

if [ "$weak" = true ] ; then
    for filename in /cfs/nobackup/s/sorme/MapReduce/bin/sorme_benchmarkers/Weak/*
    do
        sbatch ${filename}
    done
fi

if [ "$strong" = true ] ; then
    for filename in /cfs/nobackup/s/sorme/MapReduce/bin/sorme_benchmarkers/Strong/*
    do
        sbatch ${filename}
    done
fi
